// ProjectChrono, native Linux build (Qt 6 Widgets).
//
// Same launcher, same local-server.ini and same window as launcher.c. The game is a Windows
// program, so it runs inside a Wine or Proton prefix; loading the helper into it has to be done
// by a Windows process in that prefix. This build therefore keeps the UI, settings and checks
// native and runs the Windows build (ProjectChrono.exe --headless, beside this binary) inside
// the selected prefix for the launch itself, relaying its status lines.
//
// local-server.ini stays in the form the Windows build reads: GameExecutable and Certificate
// are stored as Windows paths (Z:\... for Linux files) and shown here as Linux paths.
#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringDecoder>
#include <QTimer>
#include <QUrl>
#include <QWidget>
#include <cstdio>
#include <cstdlib>
#include <functional>

static const char *kDefaultGame = "FortniteGame\\Binaries\\Win64\\FortniteClient-Win64-Shipping.exe";
static const char *kStatusTag = "@chrono ";

// ---- local-server.ini (shared with the Windows build) ----------------------------------------
struct Ini {
    QString path;
    QStringList lines;

    void load() {
        lines.clear();
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) return;
        QByteArray raw = f.readAll();
        QString text;
        if (raw.startsWith("\xff\xfe")) text = QStringDecoder(QStringDecoder::Utf16LE)(raw.mid(2));
        else text = QString::fromUtf8(raw.startsWith("\xef\xbb\xbf") ? raw.mid(3) : raw);
        lines = text.split('\n');
        for (QString &l : lines) if (l.endsWith('\r')) l.chop(1);
        while (!lines.isEmpty() && lines.last().isEmpty()) lines.removeLast();
    }
    // Index range [first, last) of the lines belonging to a section; first is -1 when it is missing.
    void section(const QString &name, int &first, int &last) const {
        first = -1; last = lines.size();
        for (int i = 0; i < lines.size(); ++i) {
            QString t = lines[i].trimmed();
            if (!t.startsWith('[')) continue;
            if (first >= 0) { last = i; return; }
            if (t.compare('[' + name + ']', Qt::CaseInsensitive) == 0) first = i + 1;
        }
    }
    int find(const QString &sec, const QString &key) const {
        int first, last; section(sec, first, last);
        if (first < 0) return -1;
        for (int i = first; i < last; ++i) {
            int eq = lines[i].indexOf('=');
            if (eq > 0 && lines[i].left(eq).trimmed().compare(key, Qt::CaseInsensitive) == 0) return i;
        }
        return -1;
    }
    QString get(const QString &sec, const QString &key, const QString &fallback = QString()) const {
        int i = find(sec, key);
        if (i < 0) return fallback;
        QString v = lines[i].mid(lines[i].indexOf('=') + 1).trimmed();
        return v.isEmpty() ? fallback : v;
    }
    void set(const QString &sec, const QString &key, const QString &value) {
        int i = find(sec, key);
        if (i >= 0) { lines[i] = key + '=' + value; return; }
        int first, last; section(sec, first, last);
        if (first < 0) { if (!lines.isEmpty()) lines << QString(); lines << '[' + sec + ']'; last = lines.size(); }
        while (last > 0 && lines[last - 1].trimmed().isEmpty() && !lines[last - 1].startsWith('[')) --last;
        lines.insert(last, key + '=' + value);
    }
    bool save() const {
        QString text = lines.join("\r\n") + "\r\n";
        bool ascii = true;
        for (QChar c : text) if (c.unicode() > 126) { ascii = false; break; }
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
        // The Windows profile API reads a file without a UTF-16 byte order mark in the ANSI code page.
        QByteArray out = ascii ? text.toLatin1()
                               : QByteArray("\xff\xfe") + QByteArray(reinterpret_cast<const char *>(text.utf16()), text.size() * 2);
        return f.write(out) == out.size();
    }
};

// ---- path forms --------------------------------------------------------------------------------
// Prefix directory that holds drive_c (a Proton prefix is <compatdata>/pfx).
static QString wineRoot(const QString &prefix) {
    if (QFileInfo(prefix + "/pfx/drive_c").isDir() || (!QFileInfo(prefix + "/drive_c").isDir() && QFileInfo(prefix + "/pfx").isDir()))
        return prefix + "/pfx";
    return prefix;
}
static QString toWindows(const QString &native, const QString &prefix) {
    if (!native.startsWith('/')) return QString(native).replace('/', '\\');   // relative to the launcher folder
    QString clean = QDir::cleanPath(native), c = QDir::cleanPath(wineRoot(prefix) + "/drive_c");
    if (!prefix.isEmpty() && clean.startsWith(c + '/')) return "C:" + clean.mid(c.size()).replace('/', '\\');
    return "Z:" + clean.replace('/', '\\');
}
static QString toNative(const QString &windows, const QString &prefix) {
    QString w = windows;
    if (w.size() >= 2 && w[1] == ':') {
        QChar drive = w[0].toLower();
        QString rest = w.mid(2).replace('\\', '/');
        if (drive == 'z') return rest.isEmpty() ? "/" : rest;
        if (drive == 'c' && !prefix.isEmpty()) return wineRoot(prefix) + "/drive_c" + rest;
        return windows;   // another drive letter: leave as typed
    }
    return w.replace('\\', '/');
}
static QString absolute(const QString &native, const QString &dir) {
    return native.startsWith('/') ? native : QDir::cleanPath(dir + '/' + native);
}

// ---- checks shared by the window and --headless -------------------------------------------------
static bool validAccount(const QString &s) {
    static const QRegularExpression re("^[A-Za-z0-9 _-]{1,32}$");
    return re.match(s).hasMatch() && s == s.trimmed();
}
static bool validIpv4(const QString &s) {
    static const QRegularExpression re("^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$");
    auto m = re.match(s);
    if (!m.hasMatch()) return false;
    for (int i = 1; i <= 4; ++i) if (m.captured(i).toInt() > 255) return false;
    return true;
}
static bool isPe64(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QByteArray dos = f.read(64);
    if (dos.size() < 64 || !dos.startsWith("MZ")) return false;
    quint32 off = quint8(dos[60]) | quint8(dos[61]) << 8 | quint8(dos[62]) << 16 | quint32(quint8(dos[63])) << 24;
    if (!f.seek(off)) return false;
    QByteArray pe = f.read(6);
    return pe.size() == 6 && pe.startsWith(QByteArray("PE\0\0", 4)) && quint8(pe[4]) == 0x64 && quint8(pe[5]) == 0x86;
}
static QString findProton() {
    QString home = QDir::homePath();
    const QStringList roots = {home + "/.steam/root", home + "/.steam/steam", home + "/.local/share/Steam",
                               home + "/.var/app/com.valvesoftware.Steam/data/Steam",
                               "/usr/share/steam", "/usr/local/share/steam"};
    for (const QString &root : roots)
        for (const char *sub : {"/compatibilitytools.d", "/steamapps/common"}) {
            QDir d(root + sub);
            const QStringList names = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
            for (const QString &name : names)
                if (name.contains("proton", Qt::CaseInsensitive) && QFileInfo(d.filePath(name + "/proton")).isFile())
                    return d.filePath(name + "/proton");
        }
    return QString();
}
static QString findSteamRoot() {
    QString home = QDir::homePath();
    for (const QString &p : {home + "/.steam/root", home + "/.steam/steam", home + "/.local/share/Steam",
                             home + "/.var/app/com.valvesoftware.Steam/data/Steam"})
        if (QFileInfo(p).isDir()) return QFileInfo(p).canonicalFilePath();
    return QString();
}

struct Settings {
    QString game, cert, server, account;      // game and cert as Linux paths (possibly relative)
    QString runner, prefix, wine, proton;     // runner: "wine" or "proton"
};
static Settings readSettings(const Ini &ini) {
    Settings s;
    s.runner = ini.get("Linux", "Runner", "wine").toLower() == "proton" ? "proton" : "wine";
    s.prefix = ini.get("Linux", "Prefix");
    if (s.prefix.isEmpty()) s.prefix = qEnvironmentVariable("WINEPREFIX", QDir::homePath() + "/.wine");
    s.wine = ini.get("Linux", "Wine");
    s.proton = ini.get("Linux", "Proton");
    s.game = toNative(ini.get("LocalServer", "GameExecutable", kDefaultGame), s.prefix);
    s.cert = toNative(ini.get("LocalServer", "Certificate", "server-ca.pem"), s.prefix);
    s.server = ini.get("LocalServer", "ServerAddress", "127.0.0.1");
    s.account = ini.get("LocalServer", "Account", "LocalPlayer");
    return s;
}
static bool writeSettings(Ini &ini, const Settings &s) {
    ini.load();
    ini.set("LocalServer", "GameExecutable", toWindows(s.game, s.prefix));
    ini.set("LocalServer", "Certificate", toWindows(s.cert, s.prefix));
    ini.set("LocalServer", "ServerAddress", s.server);
    ini.set("LocalServer", "Account", s.account);
    ini.set("Linux", "Runner", s.runner);
    ini.set("Linux", "Prefix", s.prefix);
    ini.set("Linux", "Wine", s.wine);
    ini.set("Linux", "Proton", s.proton);
    return ini.save();
}

// Builds the command that runs the Windows build inside the prefix. Returns an error text, or empty.
static QString prepareLaunch(const Ini &ini, const Settings &s, const QString &dir, QString &program, QStringList &args,
                             QProcessEnvironment &env, QString &statusFile) {
    QString stub = dir + "/ProjectChrono.exe";
    if (!validIpv4(s.server)) return "Server address must be an IPv4 address.";
    if (!validAccount(s.account)) return "Account: 1\u201332 letters, digits, spaces, underscores or hyphens.";
    if (!QFileInfo(absolute(s.game, dir)).isFile() || !QFileInfo(absolute(s.cert, dir)).isFile())
        return "Game executable or public CA certificate is missing.";
    if (!isPe64(absolute(s.game, dir))) return "Game executable must be a 64-bit Windows executable.";
    if (!QFileInfo(stub).isFile() || !QFileInfo(dir + "/FortniteLocalHelper.dll").isFile())
        return "ProjectChrono.exe and FortniteLocalHelper.dll must be beside this launcher.";
    if (s.prefix.isEmpty() || !QFileInfo(s.prefix).isDir()) return "The prefix folder does not exist.";
    env = QProcessEnvironment::systemEnvironment();
    env.insert("PROJECTCHRONO_STATUS_TAG", "1");
    if (!env.contains("WINEDEBUG")) env.insert("WINEDEBUG", "-all");
    QString overrides = ini.get("Linux", "DllOverrides");
    if (!overrides.isEmpty()) env.insert("WINEDLLOVERRIDES", overrides);
    QString prefix = QFileInfo(s.prefix).absoluteFilePath();
    static quint64 statusSeq = 0;
    statusFile = QString("%1/.projectchrono-status-%2-%3.log")
                     .arg(prefix)
                     .arg(QCoreApplication::applicationPid())
                     .arg(++statusSeq);
    QFile::remove(statusFile);
    env.insert("PROJECTCHRONO_STATUS_FILE", toWindows(statusFile, prefix));
    if (s.runner == "proton") {
        QString proton = s.proton.isEmpty() ? findProton() : s.proton;
        if (proton.isEmpty() || !QFileInfo(proton).isFile())
            return "Proton was not found. Choose the 'proton' file inside a Proton folder.";
        // Proton takes the compatdata folder, which contains pfx/; accept either being selected.
        QString compat = QDir(prefix).dirName() == "pfx" ? QFileInfo(prefix).absolutePath() : prefix;
        if (QFileInfo(compat + "/drive_c").isDir() && !QFileInfo(compat + "/pfx").isDir())
            return "This is a plain Wine prefix. Proton needs a compatdata folder (the one containing pfx).";
        QString steam = ini.get("Linux", "SteamRoot");
        if (steam.isEmpty()) steam = findSteamRoot();
        env.insert("STEAM_COMPAT_DATA_PATH", compat);
        env.insert("STEAM_COMPAT_CLIENT_INSTALL_PATH", steam.isEmpty() ? compat : steam);
        env.remove("WINEPREFIX");
        program = proton;
        args = {"run", stub, "--headless"};
    } else {
        if (!QFileInfo(wineRoot(prefix) + "/drive_c").isDir()) return "The prefix folder is not a Wine prefix (no drive_c).";
        program = s.wine.isEmpty() ? QStandardPaths::findExecutable("wine") : s.wine;
        if (program.isEmpty() || !QFileInfo(program).isFile()) return "Wine was not found. Install it or choose the wine binary.";
        env.insert("WINEPREFIX", wineRoot(prefix));
        args = {stub, "--headless"};
    }
    return QString();
}

// Runs the prepared command; status lines arrive via the prefix status file (reliable under Proton and PROTON_LOG=1)
// with tagged stderr as fallback.
class Launch : public QObject {
public:
    QProcess process;
    QTimer poll;
    QString statusFilePath;
    qint64 statusOffset = 0;
    QByteArray buffer, statusBuffer;
    bool sawStatus = false, sawFileStatus = false;
    std::function<void(const QString &)> onStatus;
    std::function<void(int)> onFinished;

    void start(const QString &program, const QStringList &args, const QProcessEnvironment &env, const QString &dir,
               const QString &statusFile) {
        statusFilePath = statusFile;
        process.setProcessEnvironment(env);
        process.setWorkingDirectory(dir);
        process.setProcessChannelMode(QProcess::MergedChannels);
        poll.setInterval(100);
        connect(&poll, &QTimer::timeout, this, [this] { drainStatusFile(false); });
        connect(&process, &QProcess::readyRead, this, [this] { buffer += process.readAll(); drain(false); });
        connect(&process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError e) {
            if (e == QProcess::FailedToStart) {
                poll.stop();
                cleanupStatusFile();
                onStatus("Could not start " + process.program() + ".");
                onFinished(1);
            }
        });
        connect(&process, &QProcess::finished, this, [this](int code, QProcess::ExitStatus st) {
            poll.stop();
            drainStatusFile(true);
            buffer += process.readAll();
            drain(true);
            cleanupStatusFile();
            if (st != QProcess::NormalExit && !code) code = 1;
            if (!sawStatus) onStatus(QString("The launcher could not run inside the prefix (exit code %1).").arg(code));
            onFinished(code);
        });
        poll.start();
        process.start(program, args);
    }
    void cleanupStatusFile() {
        if (!statusFilePath.isEmpty()) QFile::remove(statusFilePath);
    }
    void drainStatusFile(bool all) {
        if (statusFilePath.isEmpty()) return;
        QFile f(statusFilePath);
        if (!f.open(QIODevice::ReadOnly)) return;
        if (f.size() <= statusOffset) return;
        if (!f.seek(statusOffset)) return;
        QByteArray chunk = f.readAll();
        if (chunk.isEmpty()) return;
        statusOffset += chunk.size();
        statusBuffer += chunk;
        int nl;
        while ((nl = statusBuffer.indexOf('\n')) >= 0 || (all && !statusBuffer.isEmpty())) {
            QByteArray line = nl >= 0 ? statusBuffer.left(nl) : statusBuffer;
            statusBuffer.remove(0, nl >= 0 ? nl + 1 : statusBuffer.size());
            QString text = QString::fromUtf8(line).trimmed();
            if (text.isEmpty()) continue;
            sawStatus = true;
            sawFileStatus = true;
            onStatus(text);
        }
    }
    void drain(bool all) {
        int nl;
        while ((nl = buffer.indexOf('\n')) >= 0 || (all && !buffer.isEmpty())) {
            QByteArray line = nl >= 0 ? buffer.left(nl) : buffer;
            buffer.remove(0, nl >= 0 ? nl + 1 : buffer.size());
            int tag = line.indexOf(kStatusTag);
            if (tag < 0 || sawFileStatus) continue;   // Wine and Proton chatter, or already delivered via status file
            sawStatus = true;
            onStatus(QString::fromUtf8(line.mid(tag + int(strlen(kStatusTag)))).trimmed());
        }
    }
};

// ---- window --------------------------------------------------------------------------------------
static QColor rgb(int r, int g, int b) { return QColor(r, g, b); }

static int classifyStatus(const QString &msg) {
    if (msg.contains("Local helper ready")) return 2;
    if (msg.contains("failed") || msg.contains("missing") || msg.contains("must be") ||
        msg.contains("Invalid") || msg.contains("Could not") || msg.contains("timed out") ||
        msg.contains("unavailable") || msg.contains("No helper log") || msg.contains("exited with code") ||
        msg.contains("not found") || msg.contains("plain Wine prefix") || msg.contains("does not exist") ||
        msg.contains("1\u201332") || msg.contains("exited before"))
        return 1;
    if (msg.contains("\u2026") || msg.contains("...") || msg.contains("Preparing") ||
        msg.contains("Starting") || msg.contains("Loading") || msg.contains("Initializing"))
        return 3;
    return 0;
}

class Window : public QWidget {
public:
    QString dir;
    Ini ini;
    QLineEdit *game, *cert, *server, *account, *prefix, *runnerPath;
    QComboBox *runner;
    QPushButton *play, *browseButtons[4];
    QLabel *statusLabel, *runnerPathLabel, *note;
    Launch *launch = nullptr;
    QString winePath, protonPath;
    QFont base;
    int statusKind = 0;

    QFont font(int px, QFont::Weight weight = QFont::Normal) { QFont f = base; f.setPixelSize(px); f.setWeight(weight); return f; }
    QLabel *label(const QString &text, int x, int y, int w, int h, const QColor &color, int px = 14, QFont::Weight weight = QFont::Normal) {
        QLabel *l = new QLabel(text, this);
        l->setGeometry(x, y, w, h); l->setFont(font(px, weight));
        l->setStyleSheet(QString("color:%1;background:transparent").arg(color.name()));
        return l;
    }
    QLineEdit *edit(int x, int y, int w) {
        QLineEdit *e = new QLineEdit(this);
        e->setGeometry(x, y, w, 39); e->setFont(font(14));
        return e;
    }
    QPushButton *button(const QString &text, int x, int y, int w, int h, bool primary = false, bool secondarySmall = false) {
        QPushButton *b = new QPushButton(text, this);
        b->setGeometry(x, y, w, h); b->setFont(font(14, QFont::DemiBold)); b->setCursor(Qt::PointingHandCursor);
        if (primary) b->setObjectName("primary");
        else if (secondarySmall) b->setObjectName("browse");
        return b;
    }
    void setStatus(const QString &msg) {
        statusKind = classifyStatus(msg);
        QColor c = statusKind == 1 ? rgb(242, 139, 130) : statusKind == 2 ? rgb(102, 214, 204) : rgb(232, 237, 247);
        statusLabel->setStyleSheet(QString("color:%1;background:transparent").arg(c.name()));
        statusLabel->setText(msg);
        update(44, 712, 20, 20);
    }

    explicit Window(const QString &directory) : dir(directory) {
        ini.path = dir + "/local-server.ini";
        ini.load();
        setWindowTitle("ProjectChrono  |  12.60");
        setFixedSize(980, 800);
        for (const char *family : {"Inter", "Segoe UI", "Noto Sans", "Cantarell", "Ubuntu", "DejaVu Sans", "Liberation Sans"})
            if (QFontDatabase::hasFamily(family)) { base = QFont(family); break; }
        setStyleSheet(
            "QLineEdit,QComboBox{background:#273044;border:1px solid #344058;border-radius:9px;padding:0 12px;color:#f0f3fa;"
            "selection-background-color:#7e62e0}"
            "QLineEdit:focus,QComboBox:focus{border-color:#66ddd0}"
            "QLineEdit:disabled,QComboBox:disabled{color:#9499aa}"
            "QComboBox::drop-down{border:none;width:24px}"
            "QComboBox QAbstractItemView{background:#273044;color:#f0f3fa;selection-background-color:#7e62e0;border:1px solid #344058}"
            "QPushButton{background:#303a50;border:1px solid #3e4b66;border-radius:12px;color:#f2f5fc}"
            "QPushButton:hover{background:#3a465f}QPushButton:pressed{background:#3c4860}"
            "QPushButton:focus{border-color:#66ddd0}"
            "QPushButton#browse{border-radius:10px}"
            "QPushButton#primary{background:#7e62e0;border-color:#7e62e0;color:#ffffff}QPushButton#primary:hover{background:#8d73ea;border-color:#8d73ea}"
            "QPushButton#primary:pressed{background:#694fc4;border-color:#694fc4}"
            "QPushButton:disabled,QPushButton#primary:disabled,QPushButton#browse:disabled{background:#313544;border-color:#313544;color:#9499aa}");

        const QColor ink = rgb(238, 242, 251), fieldInk = rgb(212, 220, 237), secInk = rgb(143, 159, 192), dim = rgb(152, 164, 187), noteDim = rgb(139, 152, 178);
        label("PROJECTCHRONO", 48, 30, 240, 26, rgb(190, 175, 255), 19, QFont::Bold);
        label("FORTNITE 12.60   /   CHAPTER 2 \u00b7 SEASON 2", 48, 72, 420, 20, rgb(102, 214, 204), 12, QFont::Bold);
        label("Back to the island.", 48, 100, 550, 46, ink, 34, QFont::Bold);
        label("Set up your local session and make your return.", 49, 154, 570, 22, dim, 14);

        label("YOUR SESSION", 74, 228, 300, 20, secInk, 12, QFont::Bold);
        label("Server IPv4 address", 74, 266, 320, 20, fieldInk, 13, QFont::DemiBold);
        server = edit(74, 290, 364); server->setMaxLength(63);
        label("Account name", 74, 348, 320, 20, fieldInk, 13, QFont::DemiBold);
        account = edit(74, 372, 364); account->setMaxLength(32);

        label("GAME FILES", 518, 228, 300, 20, secInk, 12, QFont::Bold);
        label("Game executable", 518, 266, 320, 20, fieldInk, 13, QFont::DemiBold);
        game = edit(518, 290, 288);
        browseButtons[0] = button("Browse", 814, 290, 92, 39, false, true);
        label("Public CA certificate", 518, 348, 320, 20, fieldInk, 13, QFont::DemiBold);
        cert = edit(518, 372, 288);
        browseButtons[1] = button("Browse", 814, 372, 92, 39, false, true);

        label("LINUX RUNTIME", 74, 492, 300, 20, secInk, 12, QFont::Bold);
        label("Run with", 74, 526, 140, 20, fieldInk, 13, QFont::DemiBold);
        runner = new QComboBox(this); runner->setGeometry(74, 550, 132, 39); runner->setFont(font(14));
        runner->addItem("Wine", "wine"); runner->addItem("Proton", "proton");
        label("Prefix folder", 222, 526, 250, 20, fieldInk, 13, QFont::DemiBold);
        prefix = edit(222, 550, 250);
        browseButtons[2] = button("Browse", 478, 550, 82, 39, false, true);
        runnerPathLabel = label("", 576, 526, 330, 20, fieldInk, 13, QFont::DemiBold);
        runnerPath = edit(576, 550, 240);
        browseButtons[3] = button("Browse", 822, 550, 84, 39, false, true);

        play = button("Play  \u2192", 48, 640, 190, 50, true);
        QPushButton *log = button("Open log", 250, 640, 120, 50);
        statusLabel = label("Ready to configure your local session.", 64, 710, 868, 24, rgb(232, 237, 247), 14);
        note = label("", 48, 766, 884, 22, noteDim, 13);

        Settings s = readSettings(ini);
        game->setText(s.game); cert->setText(s.cert); server->setText(s.server); account->setText(s.account);
        prefix->setText(s.prefix); winePath = s.wine; protonPath = s.proton.isEmpty() ? findProton() : s.proton;
        runner->setCurrentIndex(s.runner == "proton" ? 1 : 0);
        showRunner();
        QTimer::singleShot(0, this, [this] {
            game->setCursorPosition(0); cert->setCursorPosition(0); prefix->setCursorPosition(0); runnerPath->setCursorPosition(0);
        });

        connect(runner, &QComboBox::currentIndexChanged, this, [this] { keepRunnerPath(!isProton()); showRunner(); });
        connect(browseButtons[0], &QPushButton::clicked, this, [this] { chooseFile(game, "Windows applications (*.exe);;All files (*)"); });
        connect(browseButtons[1], &QPushButton::clicked, this, [this] { chooseFile(cert, "PEM certificates (*.pem *.crt);;All files (*)"); });
        connect(browseButtons[2], &QPushButton::clicked, this, [this] {
            QString d = QFileDialog::getExistingDirectory(this, "Prefix folder", prefix->text());
            if (!d.isEmpty()) { prefix->setText(d); prefix->setCursorPosition(0); }
        });
        connect(browseButtons[3], &QPushButton::clicked, this, [this] { chooseFile(runnerPath, "All files (*)"); });
        connect(play, &QPushButton::clicked, this, [this] { onPlay(); });
        connect(log, &QPushButton::clicked, this, [this] {
            QString path = dir + "/ProjectChrono-helper.log";
            if (!QFileInfo(path).isFile() || !QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
                setStatus("No helper log is available yet.");
        });
    }

    bool isProton() const { return runner->currentData().toString() == "proton"; }
    void keepRunnerPath(bool wasProton) { (wasProton ? protonPath : winePath) = runnerPath->text().trimmed(); }
    void showRunner() {
        bool proton = isProton();
        runnerPathLabel->setText(proton ? "Proton script" : "Wine binary");
        runnerPath->setPlaceholderText(proton ? ".../Proton/proton" : "wine on PATH");
        runnerPath->setText(proton ? protonPath : winePath);
        runnerPath->setCursorPosition(0);
        prefix->setPlaceholderText(proton ? "compatdata folder (contains pfx)" : "Wine prefix");
        note->setText(proton ? "On Play, the selected public CA is trusted inside this Proton prefix."
                             : "On Play, the selected public CA is trusted inside this Wine prefix.");
    }
    void chooseFile(QLineEdit *target, const QString &filter) {
        QString start = target->text().isEmpty() ? dir : absolute(target->text(), dir);
        QString f = QFileDialog::getOpenFileName(this, QString(), start, filter);
        if (!f.isEmpty()) { target->setText(f); target->setCursorPosition(0); }
    }
    void setEnabledAll(bool on) {
        for (QWidget *w : {(QWidget *)game, (QWidget *)cert, (QWidget *)server, (QWidget *)account, (QWidget *)prefix,
                           (QWidget *)runnerPath, (QWidget *)runner, (QWidget *)play})
            w->setEnabled(on);
        for (QPushButton *b : browseButtons) b->setEnabled(on);
    }
    void onPlay() {
        Settings s;
        keepRunnerPath(isProton());
        s.game = game->text().trimmed(); s.cert = cert->text().trimmed();
        s.server = server->text().trimmed(); s.account = account->text();
        s.runner = isProton() ? "proton" : "wine"; s.prefix = prefix->text().trimmed();
        s.wine = winePath; s.proton = protonPath;
        if (!writeSettings(ini, s)) { setStatus("Could not save settings beside the launcher."); return; }
        QString program; QStringList args; QProcessEnvironment env; QString statusFile;
        QString problem = prepareLaunch(ini, s, dir, program, args, env, statusFile);
        if (!problem.isEmpty()) { setStatus(problem); return; }
        setEnabledAll(false);
        setStatus("Preparing local session\u2026");
        launch = new Launch;
        launch->onStatus = [this](const QString &m) { setStatus(m); };
        launch->onFinished = [this](int code) {
            if (!code) setStatus("Game process exited.");
            setEnabledAll(true);
            launch->deleteLater(); launch = nullptr;
        };
        launch->start(program, args, env, dir, statusFile);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), rgb(15, 20, 33));
        // Subtle ambient clock motif in the header.
        p.setPen(QPen(rgb(51, 59, 86), 1)); p.setBrush(QColor(126, 98, 224, 14));
        p.drawEllipse(QRectF(720.5, -64.5, 300, 300));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QRectF(747.5, -37.5, 246, 246));
        p.setPen(QPen(rgb(58, 68, 98), 1));
        p.drawEllipse(QRectF(812.5, 27.5, 116, 116));
        p.setPen(QPen(rgb(102, 214, 204), 1.8, Qt::SolidLine, Qt::RoundCap)); p.drawLine(QPointF(870.5, 85.5), QPointF(870.5, 43.5));
        p.setPen(QPen(rgb(140, 117, 216), 1.8, Qt::SolidLine, Qt::RoundCap)); p.drawLine(QPointF(870.5, 85.5), QPointF(902.5, 102.5));
        p.setPen(Qt::NoPen); p.setBrush(rgb(190, 175, 255)); p.drawEllipse(QRectF(868.0, 83.0, 5, 5));
        p.setPen(QPen(rgb(44, 54, 75), 1)); p.setBrush(rgb(27, 34, 51));
        p.drawRoundedRect(QRectF(48.5, 205.5, 416, 245), 12, 12);
        p.drawRoundedRect(QRectF(492.5, 205.5, 440, 245), 12, 12);
        p.drawRoundedRect(QRectF(48.5, 470.5, 884, 146), 12, 12);
        QColor dot = statusKind == 1 ? rgb(242, 139, 130) : statusKind == 2 ? rgb(102, 214, 204) : statusKind == 3 ? rgb(190, 175, 255) : rgb(92, 106, 135);
        p.setPen(Qt::NoPen); p.setBrush(dot); p.drawEllipse(QRectF(49.0, 718.0, 8, 8));
        p.setPen(QPen(rgb(42, 51, 70), 1)); p.drawLine(QPointF(48, 750.5), QPointF(932, 750.5));
    }
};

static int headless(const QString &dir) {
    Ini ini; ini.path = dir + "/local-server.ini"; ini.load();
    Settings s = readSettings(ini);
    QString program; QStringList args; QProcessEnvironment env; QString statusFile;
    QString problem = prepareLaunch(ini, s, dir, program, args, env, statusFile);
    if (!problem.isEmpty()) { fprintf(stderr, "%s\n", qPrintable(problem)); return 1; }
    Launch launch;
    launch.onStatus = [](const QString &m) { fprintf(stderr, "%s\n", qPrintable(m)); fflush(stderr); };
    launch.onFinished = [](int code) { QCoreApplication::exit(code); };
    launch.start(program, args, env, dir, statusFile);
    return QCoreApplication::exec();
}

int main(int argc, char **argv) {
    bool noUi = false;
    for (int i = 1; i < argc; ++i) if (!qstricmp(argv[i], "--headless")) noUi = true;
    if (noUi) {
        QCoreApplication app(argc, argv);
        return headless(QCoreApplication::applicationDirPath());
    }
    QApplication app(argc, argv);
    QApplication::setApplicationName("ProjectChrono");
    Window w(QCoreApplication::applicationDirPath());
    w.show();
    int rc = app.exec();
    // Like the Windows build, closing the launcher leaves an already running game open.
    if (w.launch) { fflush(nullptr); std::_Exit(rc); }
    return rc;
}
