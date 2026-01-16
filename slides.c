
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

const char* PART1;
const char* PART2;

#define LINE_MAX 1024
#define PATH_MAX 1024

static void skip_utf8_bom(FILE* f) {
    unsigned char bom[3];

    if (fread(bom, 1, 3, f) != 3) {
        rewind(f);
        return;
    }

    if (!(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
        rewind(f);  // not a BOM
    }
}

static void render_inline(FILE* out, const char* s) {
    while (*s) {
        if (s[0] == '*' && s[1] == '*') {
            s += 2;
            fputs("<strong>", out);
            while (*s && !(s[0] == '*' && s[1] == '*'))
                fputc(*s++, out);
            fputs("</strong>", out);
            if (*s) s += 2;
        }
        else if (*s == '*') {
            s++;
            fputs("<em>", out);
            while (*s && *s != '*')
                fputc(*s++, out);
            fputs("</em>", out);
            if (*s) s++;
        }
        else if (*s == '`') {
            s++;
            fputs("<code>", out);
            while (*s && *s != '`')
                fputc(*s++, out);
            fputs("</code>", out);
            if (*s) s++;
        }
        else {
            fputc(*s++, out);
        }
    }
}

static void escape_html(FILE* out, const char* s) {
    for (; *s; s++) {
        switch (*s) {
        case '&': fputs("&amp;", out); break;
        case '<': fputs("&lt;", out);  break;
        case '>': fputs("&gt;", out);  break;
        default:  fputc(*s, out);      break;
        }
    }
}

static void make_output_name(char* out, const char* in) {
    strcpy(out, in);
    char* dot = strrchr(out, '.');
    if (dot)
        strcpy(dot, ".html");
    else
        strcat(out, ".html");
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <file.md>\n", argv[0]);
        return 1;
    }

    char outname[PATH_MAX];
    make_output_name(outname, argv[1]);

    FILE* in = fopen(argv[1], "r");

    /* skip UTF-8 BOM if present */


    FILE* out = fopen(outname, "w");
    if (!in || !out) {
        perror("file");
        return 1;
    }

    skip_utf8_bom(in);

    fputs(PART1, out);

    char line[LINE_MAX];
    bool in_section = false;
    bool in_list = false;
    bool in_code = false;
    bool in_pre = false;
    while (fgets(line, sizeof(line), in)) {
        line[strcspn(line, "\n")] = 0;

        /* fenced code block */
        if (strcmp(line, "```c") == 0) {
            if (!in_code) {
                if (in_list) {
                    fputs("  </ul>\n", out);
                    in_list = false;
                }
                fputs("<pre><code class=\"language-c\">\n", out);
                in_code = true;
            }
            else {
                fputs("</code></pre>\n", out);
                in_code = false;
            }
            continue;
        }
        
        /*smaller font for code*/
        if (strcmp(line, "```csmall") == 0) {
            if (!in_code) {
                if (in_list) {
                    fputs("  </ul>\n", out);
                    in_list = false;
                }
                fputs("<pre><code class=\"language-c\" style=\"font-size:10px\">\n", out);
                in_code = true;
            }
            else {
                fputs("</code></pre>\n", out);
                in_code = false;
            }
            continue;
        }

        /* just pre */
        if (strcmp(line, "```") == 0) {
            if (!in_pre && !in_code) {
                if (in_list) {
                    fputs("  </ul>\n", out);
                    in_list = false;
                }
                fputs("<pre>\n", out);
                in_pre = true;
            }
            else {
                if (in_code)
                    fputs("</code>\n", out);
                fputs("</pre>\n", out);
                in_pre = false;
                in_code = false;
            }
            continue;
        }

        if (in_code) {
            escape_html(out, line);
            fputc('\n', out);
            continue;
        }

        if (in_pre) {
            fputs(line, out);
            fputc('\n', out);
            continue;
        }

        /* H2 */
        if (strncmp(line, "# ", 2) == 0) {
            if (in_list) {
                fputs("  </ul>\n", out);
                in_list = false;
            }
            if (in_section)
                fputs("</section>\n", out);

            fputs("<!---------------------------------------------------------------------------------------------->\n", out);
            fputs("<section>\n  <h1>", out);
            render_inline(out, line + 2);
            fputs("</h1>\n", out);
            in_section = true;
        }
        /* H3 */
        else if (strncmp(line, "## ", 3) == 0) {
            if (in_list) {
                fputs("  </ul>\n", out);
                in_list = false;
            }
            if (in_section)
                fputs("</section>\n", out);

            fputs("<!---------------------------------------------------------------------------------------------->\n", out);
            fputs("<section>\n  <h2>", out);
            render_inline(out, line + 2);
            fputs("</h2>\n", out);
            in_section = true;
        }
        /* H3 */
        else if (strncmp(line, "### ", 4) == 0) {
            if (in_list) {
                fputs("  </ul>\n", out);
                in_list = false;
            }
            fputs("  <h3>", out);
            render_inline(out, line + 3);
            fputs("</h3>\n", out);
        }
        else if (strncmp(line, "> ", 2) == 0) {
            if (in_list) {
                fputs("  </ul>\n", out);
                in_list = false;
            }
            fputs("  <div class=\"note\">", out);
            render_inline(out, line + 2);
            fputs("</div>\n", out);
        }
        /* unordered list */
        else if (strncmp(line, "- ", 2) == 0) {
            if (!in_list) {
                fputs("  <ul>\n", out);
                in_list = true;
            }
            fputs("    <li>", out);
            render_inline(out, line + 2);
            fputs("</li>\n", out);
        }
        /* empty line */
        else if (line[0] == '\0') {
            if (in_list) {
                fputs("  </ul>\n", out);
                in_list = false;
            }
        }
        /* paragraph */
        else {
            if (in_list) {
                fputs("  </ul>\n", out);
                in_list = false;
            }
            fputs("  <p>", out);
            render_inline(out, line);
            fputs("</p>\n", out);
        }
    }

    if (in_code)
        fputs("</code></pre>\n", out);

    if (in_pre)
        fputs("</pre>\n", out);

    if (in_list)
        fputs("  </ul>\n", out);

    if (in_section)
        fputs("</section>\n\n\n", out);

    fputs(PART2, out);

    fclose(in);
    fclose(out);
    return 0;
}

















const char* PART1
=
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <title>Local functions && Function literals</title>\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <link rel=\"stylesheet\" href=\"default.min.css\">\n"
"    <script src=\"highlight.min.js\"></script>\n"
"    <script>hljs.highlightAll();</script>\n"
"\n"
"    <style>\n"
"        /* Reset */\n"
"        * {\n"
"            margin: 0;\n"
"            padding: 0;\n"
"            box-sizing: border-box;\n"
"            font-family: Arial, Helvetica, sans-serif;\n"
"        }\n"
"\n"
"        body {\n"
"            background: white;\n"
"            color: black;\n"
"            overflow: hidden;\n"
"        }\n"
"\n"
"        pre {\n"
"            background: #f3f3f3;\n"
"            padding: 1em;\n"
"            text-align: left;\n"
"        }\n"
"\n"
"        /* Slide container */\n"
"        .presentation {\n"
"            width: 100vw;\n"
"            height: 100vh;\n"
"            position: relative;\n"
"            \n"
"        }\n"
"\n"
"        /* Individual slide */\n"
"        section {\n"
"            width: 100%;\n"
"            height: 100%;\n"
"            padding: 60px;\n"
"            display: none;\n"
"            flex-direction: column;\n"
"            justify-content: center;\n"
"            max-width: 50em;\n"
"            margin: auto;\n"
"        }\n"
"\n"
"        section.active {\n"
"            display: flex;\n"
"        }\n"
"\n"
"\n"
"\n"
"        /* Titles */\n"
"        section h1 {\n"
"            font-size: 3rem;\n"
"            margin-bottom: 20px;\n"
"            text-align: center;\n"
"        }\n"
"\n"
"        section h2 {\n"
"            font-size: 2rem;\n"
"            margin-bottom: 15px;\n"
"            text-align: center;\n"
"        }\n"
"\n"
"        /* Text */\n"
"        section li {\n"
"            font-size: 1.3rem;\n"
"            line-height: 1.6;\n"
"        }\n"
"\n"
"        ul {\n"
"            margin-left: 30px;\n"
"        }\n"
"\n"
"\n"
"\n"
"        /* Footer */\n"
"        .footer {\n"
"            position: absolute;\n"
"            bottom: 20px;\n"
"            right: 30px;\n"
"            font-size: 0.9rem;\n"
"            opacity: 0.6;\n"
"        }\n"
"\n"
"        .note {\n"
"            background-color: #ffffff;\n"
"            color: #222;\n"
"            border: 1px solid #e5e5e5;\n"
"            border-left: 4px solid #d0d0d0;\n"
"\n"
"            padding: 8px 10px;\n"
"            margin: 8px 0;\n"
"\n"
"            border-radius: 6px;\n"
"\n"
"            font-family: system-ui, -apple-system, BlinkMacSystemFont, \"Segoe UI\", sans-serif;\n"
"            font-size: smaller;\n"
"            line-height: 1.6;\n"
"\n"
"            box-shadow: 0 1px 2px rgba(0, 0, 0, 0.04);\n"
"        }\n"
"\n"
"\n"
"    </style>\n"
"\n"
"\n"
"</head>\n"
"\n"
"<body>\n"
"\n"
"    <div class=\"presentation\">";


const char* PART2
 =
 "        <!--PART2-->\n"
 "\n"
 "        <div class=\"footer\">Slide N/N</div>\n"
 "    </div>\n"
 "\n"
 "    <script>\n"
 "        const start_time = Date.now();\n"
 "        const slides = document.querySelectorAll(\"section\");\n"
 "        let currentSlide = 0;\n"
 "\n"
 "        function showSlide(index)\n"
 "        {\n"
 "\n"
 "            const end_time = Date.now();\n"
 "            const duration_minutes = Math.round((end_time - start_time) / 60000);\n"
 "            const total = slides.length;\n"
 "            var footer = document.querySelector(\".footer\");\n"
 "            if (footer)\n"
 "            {\n"
 "                footer.textContent = `Slide ${index + 1} / ${total}`;\n"
 "            }\n"
 "\n"
 "            slides.forEach((slide, i) =>\n"
 "            {\n"
 "                slide.classList.remove(\"active\");\n"
 "            });\n"
 "\n"
 "            slides[index].classList.add(\"active\");\n"
 "        }\n"
 "        function nextSlide()\n"
 "        {\n"
 "            currentSlide = Math.min(currentSlide + 1, slides.length - 1);\n"
 "            showSlide(currentSlide);\n"
 "\n"
 "        }\n"
 "        function previousSlide()\n"
 "        {\n"
 "            currentSlide = Math.max(currentSlide - 1, 0);\n"
 "            showSlide(currentSlide);\n"
 "\n"
 "        }\n"
 "\n"
 "\n"
 "        // Mouse click\n"
 "        document.addEventListener(\"click\", (e) =>\n"
 "        {\n"
 "            const width = window.innerWidth;\n"
 "\n"
 "            if (x < width / 2)\n"
 "            {\n"
 "                previousSlide();\n"
 "            } else\n"
 "            {\n"
 "                nextSlide();\n"
 "            }\n"
 "        });\n"
 "\n"
 "\n"
 "        document.addEventListener(\"keydown\", (e) =>\n"
 "        {\n"
 "            if (e.key === \"ArrowRight\" || e.key === \" \")\n"
 "            {\n"
 "                nextSlide();\n"
 "            }\n"
 "\n"
 "            if (e.key === \"ArrowLeft\")\n"
 "            {\n"
 "                previousSlide();\n"
 "            }\n"
 "        });\n"
 "\n"
 "        // Initialize first slide\n"
 "        showSlide(currentSlide);\n"
 "    </script>\n"
 "</body>\n"
 "\n"
 "";
