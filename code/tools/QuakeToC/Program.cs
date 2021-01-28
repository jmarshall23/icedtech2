﻿using System;
using System.IO;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace QuakeToC
{
    class Program
    {
        static string LookupTableText = "";
        static string LookupFuncList = "";
        static int LookupNumFuncs = 0;
        static List<string> animEventNames = new List<string>();

        static void WriteHeader(QCfile qc, string filename)
        {
            string text = "// This file has been auto generated by the QuakeToC conversion tool\n";
            text += "// \n\n";

            int i = 0;
            foreach(QCfile.AnimationListEntry anim in qc.animations)
            {
                foreach (string s in anim.animations)
                {
                    text += "#define " + s + " " + i + "\n";
                    i++;
                }
            }

            foreach(QCfile.Function f in qc.functions)
            {
                if(f.name.Contains("touch") || f.name.Contains("Touch"))
                {
                    string func = "void " + f.name + "(gentity_t *self, gentity_t *other, trace_t *trace);\n";
                    text += func;
                    LookupFuncList += func;

                    LookupTableText += "\t{ \"" + f.name + "\", NULL, " + f.name + ", NULL },\n";
                }
                else if (!f.hasReturnType)
                {
                    string func = "void " + f.name + "(gentity_t *self);\n";
                    text += func;
                    LookupFuncList += func;
                    LookupTableText += "\t{ \"" + f.name + "\"," + f.name + ", NULL, NULL },\n";
                }
                else
                {
                    string func = "float " + f.name + "(gentity_t *self);\n";
                    text += func;
                    LookupFuncList += func;
                    LookupTableText += "\t{ \"" + f.name + "\", NULL, NULL," + f.name + " },\n";
                }

                LookupNumFuncs++;
            }

            File.WriteAllText(filename, text);
        }

        static string PostFixFunctionBody(QCfile qc, string body)
        {
            foreach (QCfile.Function f in qc.functions)
            {
                body = body.Replace(f.name + "()", f.name + "(self)");
                body = body.Replace(f.name + " ()", f.name + "(self)");
            }

            body = body.Replace("walkmove(", "walkmove(self, ");
            body = body.Replace("walkmove (", "walkmove(self, ");            

            body = body.Replace("= true", "= qtrue");
            body = body.Replace("= false", "= qfalse");

            body = body.Replace("FireBullets(", "FireBullets(self, ");
            body = body.Replace("FireBullets (", "FireBullets(self, ");
            body = body.Replace(".velocity", "->velocity");
            body = body.Replace("DropBackpack()", "DropBackpack(self)");
            body = body.Replace("walkmonster_start();", "walkmonster_start(self);");
            body = body.Replace("walkmonster_start ();", "walkmonster_start(self);");
            body = body.Replace("self->solid = SOLID_NOT;", "\tself->r.contents = 0;\n\ttrap_LinkEntity(self);\n");
            body = body.Replace("self->solid = SOLID_SLIDEBOX;", "\tself->r.contents = CONTENTS_SOLID;\n\ttrap_LinkEntity(self);\n");
            return body;
        }

        static void WriteClass(QCfile qc, string filename)
        {
            string text = "// This file has been auto generated by the QuakeToC conversion tool\n";
            text += "// \n\n";

            string header = Path.GetFileNameWithoutExtension(filename) + ".h";

            text += "#include \"../../game/g_local.h \"\n";
            text += "#include \"../superscript.h\"\n";
            text += "extern \"C\" {\n";
            text += "\t#include \"" + header + "\"\n\n";
            text += "};\n";

            foreach (QCfile.Variable v in qc.globalVariables)
            {
                text += v.type + " " + v.name + " = 0;\n";
            }

            text += "\n";

            foreach (QCfile.Function f in qc.functions)
            {
                if(f.IsAnimation)
                {
                    if (f.ai_function == null || f.ai_function.Length <= 0)
                    {
                        f.ai_function = "ai_stand";
                        f.ai_param = "0";
                        text += "// WARNING: No AI state change specified, defaulting to ai_stand\n";
                    }

                    if (f.name.Contains("touch") || f.name.Contains("Touch"))
                    {
                        text += "void " + f.name + "(gentity_t *self, gentity_t *other, trace_t *trace) {\n";
                    }
                    else if (!f.hasReturnType)
                    {
                        text += "void " + f.name + "(gentity_t *self) {\n";
                    }
                    else
                    {
                        text += "float " + f.name + "(gentity_t *self) {\n";
                    }
                    text += "\tstatic mframe_t frame = { " + f.ai_function + "," + f.ai_param + ", NULL" + " };\n";
                    text += "\tstatic mmove_t move = { " + f.anim_frame + ", " + f.anim_frame + ", &frame, " + f.next_anim + "};\n";
                    text += "\tself->monsterinfo.currentmove = &move;\n";

                    // Check to see if this is a start of a animation.
                    QCfile.AnimationListEntry anim = null;
                    for(int l = 0; l < qc.animations.Count; l++)
                    {
                        if(f.anim_frame == qc.animations[l].animations[0])
                        {
                            anim = qc.animations[l];
                            break;
                        }
                    }

                    if(anim != null)
                    {
                        string animEventName = Regex.Replace(anim.animations[0], @"[\d-]", string.Empty); // Remove all numbers from the animation name to create a "event" name. 
                        string finalAnimEventName = "anim_event_" + animEventName;

                        text += "// Anim Start function\n";
                        text += "G_SendAnimUpdate(self, " + finalAnimEventName + "," + anim.animations[0] + "," + anim.animations[anim.animations.Count - 1] + ");\n";

                        if(!animEventNames.Contains(finalAnimEventName))
                        {
                            animEventNames.Add(finalAnimEventName);
                        }
                    }

                    text += PostFixFunctionBody(qc, f.function_body);
                    text += "}\n";
                }
                else
                {
                    if (f.name.Contains("touch") || f.name.Contains("Touch"))
                    {
                        text += "void " + f.name + "(gentity_t *self, gentity_t *other, trace_t *trace) {\n";
                    }
                    else if (!f.hasReturnType)
                    {
                        text += "void " + f.name + "(gentity_t *self) {\n";
                    }
                    else
                    {
                        text += "float " + f.name + "(gentity_t *self) {\n";
                    }
                    text += PostFixFunctionBody(qc, f.function_body);
                    text += "}\n";
                }
            }

            File.WriteAllText(filename, text);
        }

        static bool ShouldBuild(string f, string name)
        {
            if (!File.Exists("../superscript/generated/generated_" + name + ".cpp"))
                return true;

            if (!File.Exists("../superscript/generated/generated_" + name + ".h"))
                return true;

            FileInfo file = new FileInfo(f);
            FileInfo destFile = new FileInfo("../superscript/generated/generated_" + name + ".cpp");

            return file.LastWriteTime > destFile.LastWriteTime;
        }

        static void Main(string[] args)
        {
            string[] files = Directory.GetFiles("../superscript/", "*.qc*");

            string LookupTableTextBody = "// This file has been auto generated by the QuakeToC conversion tool\n//\n\n";

            LookupTableTextBody += "typedef struct {\n";
            LookupTableTextBody += "\tconst char *func_name;\n";            
            LookupTableTextBody += "\tvoid (*func1)(gentity_t *self);\n";
            LookupTableTextBody += "\tvoid (*func2)(gentity_t *self, gentity_t *other, trace_t *trace);\n";
            LookupTableTextBody += "\tfloat (*func3)(gentity_t *self);\n";
            LookupTableTextBody += "} funcTable_t;\n";            

            int newFilesUpdated = 0;

            foreach (string f in files)
            {
                string name = Path.GetFileNameWithoutExtension(f);

                if (!ShouldBuild(f, name))
                    continue;

                newFilesUpdated++;
            }

            if(newFilesUpdated == 0)
            {
                Console.WriteLine("No scripts need to be updated");
                return;
            }

            foreach (string f in files)
            {               
                string name = Path.GetFileNameWithoutExtension(f);

                // Load and parse the QC file.
                QCfile qc = new QCfile(f);

                Console.WriteLine("Compiling " + f);

               // LookupTableTextBody += "#include \"generated_" + name + ".h\"\n";

                // Write out the animation header.
                WriteHeader(qc, "../superscript/generated/generated_" + name + ".h");

                WriteClass(qc, "../superscript/generated/generated_" + name + ".cpp");
            }

            LookupTableTextBody += LookupFuncList;

            if (newFilesUpdated > 0)
            {
                LookupTableTextBody += "#define NUM_SUPERSCRIPT_FUNCS " + LookupNumFuncs + "\n";
                LookupTableTextBody += "static funcTable_t superScriptTable[] = {\n";
                LookupTableTextBody += LookupTableText;
                LookupTableTextBody += "};\n";
                File.WriteAllText("../superscript/generated/save_func.h", LookupTableTextBody);

                string anim_public_header = "// This file has been auto generated by the QuakeToC conversion tool\n//\n\n";
                anim_public_header += "typedef enum {\n";
                anim_public_header += "\tanim_event_none = 0,\n";
                for(int d = 0; d < animEventNames.Count; d++)
                {
                    anim_public_header += "\t" + animEventNames[d] + ",\n";
                }
                
                anim_public_header += "} animEventType_t;\n";
                File.WriteAllText("../superscript/generated/anim_public.h", anim_public_header);
            }
        }
    }
}