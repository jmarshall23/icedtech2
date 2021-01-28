using System;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.IO;

namespace QuakeToC
{
    public class QCfile
    {
        string[] tokens;

        public string directory;
        public float[] origin = new float[3];
        public string _base;
        public string _skin;

        public class Variable
        {
            public string type;
            public string name;
        }

        public class AnimationListEntry
        {
            public List<string> animations = new List<string>();
            public int startAnimVal;
        }

        public List<Variable> globalVariables = new List<Variable>();
        public List<AnimationListEntry> animations = new List<AnimationListEntry>();
        public List<Function> functions = new List<Function>();

        public class Function
        {
            public bool IsAnimation
            {
                get
                {
                    return anim_frame != null && anim_frame.Length > 0;
                }
            }

            public string parms = "";
            public string name;
            public string anim_frame;
            public string next_anim;
            public string function_body;

            public string ai_function;
            public string ai_param;

            public bool hasReturnType;
        }

        /// <summary>
        /// Splits the given string into a list of substrings, while outputting the splitting
        /// delimiters (each in its own string) as well. It's just like String.Split() except
        /// the delimiters are preserved. No empty strings are output.</summary>
        /// <param name="s">String to parse. Can be null or empty.</param>
        /// <param name="delimiters">The delimiting characters. Can be an empty array.</param>
        /// <returns></returns>
        private string[] SplitAndKeepDelimiters(string s, params char[] delimiters)
        {
            var parts = new List<string>();
            if (!string.IsNullOrEmpty(s))
            {
                int iFirst = 0;
                do
                {
                    int iLast = s.IndexOfAny(delimiters, iFirst);
                    if (iLast >= 0)
                    {
                        if (iLast > iFirst)
                            parts.Add(s.Substring(iFirst, iLast - iFirst)); //part before the delimiter
                        parts.Add(new string(s[iLast], 1));//the delimiter
                        iFirst = iLast + 1;
                        continue;
                    }

                    //No delimiters were found, but at least one character remains. Add the rest and stop.
                    parts.Add(s.Substring(iFirst, s.Length - iFirst));
                    break;

                } while (iFirst < s.Length);
            }

            return parts.ToArray();
        }

        public QCfile(string filename)
        {
            Console.WriteLine("Parsing " + filename);
            Compile(File.ReadAllText(filename));
        }

        private string GetNextToken(string[] tokens, ref int i, bool allowEOF = false)
        {
            while (i < tokens.Length)
            {
                if (tokens[i][0] == ' ' || tokens[i][0] == '\t' || tokens[i][0] == '\n' || tokens[i][0] == '\r')
                {
                    i++;
                    continue;
                }

                return tokens[i++];
            }

            if(!allowEOF)
                throw new Exception("End of file found!");
            return null;
        }

        private void ExpectToken(string expected, string[] tokens, ref int i)
        {
            string t = GetNextToken(tokens, ref i);
            if (t != expected)
                throw new Exception("Expected token " + expected + " but found " + t);
        }

        public string ParseBracketText(Function func, string[] tokens, ref int i)
        {
            string bracketText = "";
            int bracket = 1;

            ExpectToken("{", tokens, ref i);

            while (true)
            {
                string t = tokens[i++];

                if (t == "{")
                {
                    bracket++;
                }
                else if(t == "}")
                {
                    bracket--;

                    if (bracket == 0)
                        break;
                }

                if (t == "local")
                    continue;

                string ai_func = t.ToLower();
                if(ai_func == "ai_melee" ||  ai_func == "ai_charge_side" || ai_func == "ai_melee_side" || ai_func == "ai_forward" || ai_func == "ai_walk" || ai_func == "ai_run" || ai_func == "ai_stand" || ai_func == "ai_turn" || ai_func == "ai_move" || ai_func == "ai_face" || ai_func == "ai_painforward" || ai_func == "ai_back" || ai_func == "ai_pain" || ai_func == "ai_charge")
                {
                    if(ai_func == "ai_forward")
                    {
                        func.ai_function = "ai_move";
                    }
                    else
                    {
                        func.ai_function = ai_func;
                    }
                    
                    ExpectToken("(", tokens, ref i);

                    if (t != "ai_stand" && t != "ai_face")
                    {
                        func.ai_param = GetNextToken(tokens, ref i);
                    }
                    else
                    {
                        func.ai_param = "0";
                    }
                    ExpectToken(")", tokens, ref i);
                    ExpectToken(";", tokens, ref i);
                    continue;
                }

                if(t == "time")
                {
                    t = "level.time";
                }
                else if(t == "vector")
                {
                    t = "vec3_t";
                }
                else if(t == "entity")
                {
                    t = "gentity_t *";
                }

                else if (t == "normalize")
                {
                    t = "NormalizeQuakeC";
                }

                t = t.Replace("checkbottom", "M_CheckBottom");

                t = t.Replace("SUB_Null", "nullptr");                

                t = t.Replace("vlen", "VectorLength");

                t = t.Replace(".mins_x", "->r.mins[0]");
                t = t.Replace(".mins_y", "->r.mins[1]");
                t = t.Replace(".mins_z", "->r.mins[2]");

                t = t.Replace(".maxs_x", "->r.maxs[0]");
                t = t.Replace(".maxs_y", "->r.maxs[1]");
                t = t.Replace(".maxs_z", "->r.maxs[2]");                

                t = t.Replace(".origin_x", "->r.currentOrigin[0]");
                t = t.Replace(".origin_y", "->r.currentOrigin[1]");
                t = t.Replace(".origin_z", "->r.currentOrigin[2]");
                t = t.Replace(".origin", "->r.currentOrigin");
                t = t.Replace("self.", "self->");
                t = t.Replace("other.", "other->");

                t = t.Replace("th_checkattack", "monsterinfo.checkattack");
                t = t.Replace("th_melee", "monsterinfo.melee");
                t = t.Replace("th_stand", "monsterinfo.stand");
                t = t.Replace("th_walk", "monsterinfo.walk");
                t = t.Replace("th_run", "monsterinfo.run");
                t = t.Replace("th_stand", "monsterinfo.stand");
                t = t.Replace("th_missile", "monsterinfo.attack");
                t = t.Replace("th_pain", "monsterinfo.pain");
                t = t.Replace("th_die", "monsterinfo.die");

                t = t.Replace("->angles", "->r.currentAngles");
                t = t.Replace("currentAngles_x", "currentAngles[0]");
                t = t.Replace("currentAngles_y", "currentAngles[1]");
                t = t.Replace("currentAngles_z", "currentAngles[2]");

                bracketText += t;
            }

            ExpectToken(";", tokens, ref i);

            return bracketText;
        }

        private void ParseFunction(string[] tokens, ref int i, bool hasReturnType)
        {
            Function func = new Function();

            func.hasReturnType = hasReturnType;

            // Parse the parametor list.
            ExpectToken("(", tokens, ref i);
            while(true)
            {
                string token = GetNextToken(tokens, ref i);

                if (token == ")")
                    break;
                
                func.parms += token;
            }
            i++;

            func.name = GetNextToken(tokens, ref i);

            // Test to see if this is a function decleration.
            {
                int test = i;
                string temp = GetNextToken(tokens, ref i);
                if(temp == ";")
                {
                    return;
                }
                i = test;
            }

            int currentToken = i;

            if(GetNextToken(tokens, ref i) == "=")
            {
                currentToken = i;
                if (GetNextToken(tokens, ref i) == "[")
                {
                    func.anim_frame = GetNextToken(tokens, ref i).Substring(1);
                    ExpectToken(",", tokens, ref i);
                    func.next_anim = GetNextToken(tokens, ref i);
                    ExpectToken("]", tokens, ref i);
                }
                else
                {
                    i = currentToken;
                }
            }
            else
            {
                i = currentToken;
            }

            func.function_body = ParseBracketText(func, tokens, ref i);
            functions.Add(func);
        }

        private void Compile(string data)
        {
            tokens = SplitAndKeepDelimiters(StripComments(data), '\n', '\r', '(', ')', '\t', ' ', '{', '}', '=', '[', ']', ',', ';');

            int i = 0;
            int numTotalFrames = 0;

            while(i < tokens.Length)
            {
                string t = GetNextToken(tokens, ref i, true);

                if (t == null)
                    break;

                if (t == "$cd")
                {
                    directory = GetNextToken(tokens, ref i);
                }
                else if (t == "$origin")
                {
                    origin[0] = float.Parse(GetNextToken(tokens, ref i));
                    origin[1] = float.Parse(GetNextToken(tokens, ref i));
                    origin[2] = float.Parse(GetNextToken(tokens, ref i));
                }
                else if (t == "$base")
                {
                    _base = GetNextToken(tokens, ref i);
                }
                else if (t == "$skin")
                {
                    _base = GetNextToken(tokens, ref i);
                }
                else if (t == "$frame")
                {
                    AnimationListEntry entry = new AnimationListEntry();
                    while (tokens[i][0] != '\n' && tokens[i][0] != '\r')
                    {
                        if (tokens[i][0] == ' ' || tokens[i][0] == '\t')
                        {
                            i++;
                            continue;
                        }

                        if(entry.animations.Count == 0)
                        {
                            entry.startAnimVal = numTotalFrames;
                        }

                        entry.animations.Add(tokens[i++]);
                        numTotalFrames++;
                    }

                    animations.Add(entry);
                }
                else if(t == "void")
                {
                    ParseFunction(tokens, ref i, false);
                }
                else if (t == "float")
                {
                    ParseFunction(tokens, ref i, true);
                }
                else if(t[0] == '.')
                {
                    Variable v = new Variable();
                    v.type = t.Substring(1);
                    v.name = GetNextToken(tokens, ref i);
                    globalVariables.Add(v);
                    ExpectToken(";", tokens, ref i);
                }
                else
                {
                    throw new Exception("Unknown token " + t);
                }
            }

            Console.WriteLine("Found " + animations.Count + " animations");
            Console.WriteLine("Found " + functions.Count + " functions");
        }

        static string StripComments(string code)
        {
            var re = @"(@(?:""[^""]*"")+|""(?:[^""\n\\]+|\\.)*""|'(?:[^'\n\\]+|\\.)*')|//.*|/\*(?s:.*?)\*/";
            return Regex.Replace(code, re, "$1");
        }
    }
}
