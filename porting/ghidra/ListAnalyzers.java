// Ghidra GhidraScript (Java). Lists registered analyzers and the current analysis-option values.
// Read-only: opens no transaction and writes nothing to the program.
// @category Fortnite
import java.util.List;

import ghidra.app.script.GhidraScript;
import ghidra.app.services.Analyzer;
import ghidra.framework.options.Options;
import ghidra.program.model.listing.Program;
import ghidra.util.classfinder.ClassSearcher;

public class ListAnalyzers extends GhidraScript {
    @Override
    protected void run() throws Exception {
        Options opts = currentProgram.getOptions(Program.ANALYSIS_PROPERTIES);
        println("PROGRAM " + currentProgram.getName());
        println("ANALYSIS_PROPERTIES option names (" + opts.getOptionNames().size() + "):");
        for (String name : opts.getOptionNames()) {
            println("  OPT " + name + " = " + opts.getValueAsString(name));
        }
        println("ANALYZERS:");
        List<Analyzer> all = ClassSearcher.getInstances(Analyzer.class);
        for (Analyzer a : all) {
            String def;
            String can;
            try { def = String.valueOf(a.getDefaultEnablement(currentProgram)); } catch (Exception e) { def = "err:" + e; }
            try { can = String.valueOf(a.canAnalyze(currentProgram)); } catch (Exception e) { can = "err:" + e; }
            String cur = opts.isRegistered(a.getName()) ? opts.getValueAsString(a.getName()) : "unregistered";
            println("  AN " + a.getName() + " | class=" + a.getClass().getSimpleName() + " | type="
                + a.getAnalysisType() + " | default=" + def + " | canAnalyze=" + can + " | current=" + cur);
        }
    }
}
