// Ghidra GhidraScript (Java), used as -preScript for the slim analysis of the runtime dump.
// Turns analysis options off before analyzeHeadless runs auto-analysis. Keeps the name-recovery
// analyzers on (PE exception handling -> functions, Disassemble Entry Points, Reference/Data
// Reference, Subroutine References, Non-Returning Known) and disables the slow or unneeded ones.
// AddressTableAnalyzer ("Create Address Tables") is off: it ran out of heap in the full run.
// @category Fortnite
import java.util.LinkedHashMap;
import java.util.Map;

import ghidra.app.script.GhidraScript;
import ghidra.framework.options.Options;
import ghidra.program.model.listing.Program;

public class SlimAnalysisOptions extends GhidraScript {
    // option name -> desired value (false = analyzer or sub-option off)
    private static final String[] OFF = {
        "Create Address Tables",                    // AddressTableAnalyzer (OOM in the full run)
        "Data Reference.Create Address Tables",
        "Reference.Create Address Tables",
        "Decompiler Parameter ID",                  // DecompilerFunctionAnalyzer
        "Decompiler Switch Analysis",               // DecompilerSwitchAnalyzer
        "Apply Data Archives",                      // ApplyDataArchiveAnalyzer
        "Function ID",                              // FidAnalyzer
        "Stack",                                    // StackVariableAnalyzer
        "Windows x86 PE RTTI Analyzer",             // RttiAnalyzer
        "Non-Returning Functions - Discovered",     // FindNoReturnFunctionsAnalyzer
        "Function Start Search",                    // FunctionStart* (.pdata gives the functions)
        "x86 Constant Reference Analyzer",          // X86Analyzer (RIP-relative refs come from Reference)
        "Call Convention ID",                       // DecompilerCallConventionAnalyzer
        "ASCII Strings",                            // StringsAnalyzer
        "Embedded Media",
        "Windows x86 Thread Environment Block (TEB) Analyzer",
        "Aggressive Instruction Finder",
    };
    private static final String[] ON = {
        "Windows x86 PE Exception Handling",        // PEExceptionAnalyzer (.pdata -> functions)
        "Disassemble Entry Points",                 // EntryPointAnalyzer
        "Reference",                                // OperandReferenceAnalyzer
        "Data Reference",                           // DataOperandReferenceAnalyzer
        "Subroutine References",                    // FunctionAnalyzer, CreateThunkAnalyzer
        "Non-Returning Functions - Known",          // NoReturnFunctionAnalyzer
        "Scalar Operand References",
        "External Entry References",
        "Call-Fixup Installer",
    };

    @Override
    protected void run() throws Exception {
        Options opts = currentProgram.getOptions(Program.ANALYSIS_PROPERTIES);
        int tx = currentProgram.startTransaction("slim analysis options");
        boolean ok = false;
        try {
            for (String name : OFF) {
                if (opts.isRegistered(name)) {
                    opts.setBoolean(name, false);
                    println("SLIM OFF " + name);
                } else {
                    println("SLIM MISSING " + name);
                }
            }
            for (String name : ON) {
                if (opts.isRegistered(name)) {
                    opts.setBoolean(name, true);
                    println("SLIM ON  " + name);
                } else {
                    println("SLIM MISSING " + name);
                }
            }
            ok = true;
        } finally {
            currentProgram.endTransaction(tx, ok);
        }
        println("SLIM options applied to " + currentProgram.getName());
    }
}
