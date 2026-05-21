/*
Two pass assembler for SIMPLEX instruction set
Name : E V Sai Chathurya
Roll No. : 2401CS84
Declaration of Authorship : This program is my own work and has not been copied from any source
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

#define MAX_LINE 512      // max characters per line
#define MAX_TOKEN 64      // max characters in a label / mnemonic
#define MAX_LABELS 1024      // max labels
#define MAX_CODE 8192      // max machine instructions

// Instruction structure
typedef struct {
    const char *mnemonic;
    int opcode;
    bool has_operand;
    bool is_branch;
} Instr;

// Instruction table
static const Instr INSTR_TABLE[] = {
    { "ldc",    0,  true,  false },
    { "adc",    1,  true,  false },
    { "ldl",    2,  true,  false },
    { "stl",    3,  true,  false },
    { "ldnl",   4,  true,  false },
    { "stnl",   5,  true,  false },
    { "add",    6,  false, false },
    { "sub",    7,  false, false },
    { "shl",    8,  false, false },
    { "shr",    9,  false, false },
    { "adj",    10, true,  false },
    { "a2sp",   11, false, false },
    { "sp2a",   12, false, false },
    { "call",   13, true,  true  },
    { "return", 14, false, false },
    { "brz",    15, true,  true  },
    { "brlz",   16, true,  true  },
    { "br",     17, true,  true  },
    { "HALT",   18, false, false },
    { "data",   -1, true,  false }, // pseudo-instructions  (opcode -1)
    { "SET",    -1, true,  false },
    { NULL,      0, false, false }   
};

// Label structure
typedef struct {
    char name[MAX_TOKEN];
    int value;     // word address or constant assigned by SET
    bool used;      // set true the first time this label is referenced
} Label;

static Label label_table[MAX_LABELS]; // Label table storing all labels
static int label_count = 0; // Total no. of labels stored

// Stores generated machine instructions
static int32_t obj_code[MAX_CODE];   // encoded 32-bit words
static int obj_count = 0;        // number of words stored

static int error_count   = 0; // no. of errors
static int warning_count = 0; // no. of warnings
static FILE *log_fp = NULL; // used to write errors and warnings 

// Case-insensitive comparison
static int cmp(const char *a, const char *b) {
    while (*a && *b) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

// Prints the error and writes into the log file
static void report_error(int lineno, const char *msg) {
    fprintf(stderr, "ERROR   line %d: %s\n", lineno, msg);
    if (log_fp) fprintf(log_fp, "ERROR   line %d: %s\n", lineno, msg);
    error_count++;
}

// Prints the warning and writes into the log file 
static void report_warning(int lineno, const char *msg) {
    fprintf(stderr, "WARNING line %d: %s\n", lineno, msg);
    if (log_fp) fprintf(log_fp, "WARNING line %d: %s\n", lineno, msg);
    warning_count++;
}

// Skip spaces and tabs, return updated pointer
static const char *skip(const char *p) {
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

// Label-table operations
static int label_find(const char *name) {
    for (int i = 0; i < label_count; i++)
        if (strcmp(label_table[i].name, name) == 0) return i;
    return -1;
}

// Adds a new label during pass = 1, returns false if the label already exists
static bool label_add(const char *name, int value) {
    if (label_find(name) >= 0) return false;
    if (label_count >= MAX_LABELS) {
        fprintf(stderr, "FATAL: label table overflow\n");
        exit(EXIT_FAILURE);
    }
    Label *lp = &label_table[label_count++];
    snprintf(lp->name, MAX_TOKEN, "%s", name);
    lp->value = value;
    lp->used  = false;
    return true;
}

// *ok = true if label found. Marks label used , return label address
static int label_value(const char *name, bool *ok) {
    int i = label_find(name);
    if (i < 0) { *ok = false; return 0; }
    label_table[i].used = true;
    *ok = true;
    return label_table[i].value;
}

// Find label name from address
static int label_at_addr(int addr) {
    for (int i = 0; i < label_count; i++)
        if (label_table[i].value == addr) return i;
    return -1;
}

// Valid label name
static bool valid_label_name(const char *s) {
    if (!isalpha((unsigned char)*s) && *s != '_') return false;
    for (s++; *s; s++)
        if (!isalnum((unsigned char)*s) && *s != '_') return false;
    return true;
}

// Locate instruction in the table
static const Instr *instr_find(const char *mnem) {
    for (const Instr *p = INSTR_TABLE; p->mnemonic; p++)
        if (cmp(p->mnemonic, mnem) == 0) return p;
    return NULL;
}

// Operand parsing 
static bool parse_operand(const char *token, int pc, bool is_branch,
                           int pass, int lineno, int *out) {
    // Token is number
    char *endp;
    long num = strtol(token, &endp, 0);
    if (endp != token && *endp == '\0') {
        *out = (int)num;
        return true;
    }

    //  Token is a label name 
    if (valid_label_name(token)) {
        bool ok;
        int lval = label_value(token, &ok);
        if (!ok) {
            if (pass == 2) {
                char buf[MAX_LINE];
                snprintf(buf, sizeof buf, "undefined label '%s'", token);
                report_error(lineno, buf);
                return false;
            }
            *out = 0;   // pass = 1 , return 0 - no error
            return true;
        }
        *out = is_branch ? (lval - (pc + 1)) : lval;
        return true;
    }

    //  Token is neither valid number nor valid label 
    char buf[MAX_LINE];
    snprintf(buf, sizeof buf, "invalid operand '%s'", token);
    report_error(lineno, buf);
    return false;
}

//  Instruction encoding:- bits 0-7 : opcode ,bits 8-31 : operand (signed 24-bit)
static int32_t encode_word(int opcode, int operand) {
    uint32_t uop  = (uint32_t)(opcode  & 0xFF);
    uint32_t uopr = (uint32_t)(operand & 0x00FFFFFF);
    return (int32_t)((uopr << 8) | uop);
}

// Adds the generated machine instruction to the object code array
static void emit_word(int32_t word) {
    if (obj_count >= MAX_CODE) {
        fprintf(stderr, "FATAL: object-code buffer overflow\n");
        exit(EXIT_FAILURE);
    }
    obj_code[obj_count++] = word;
}

// Build an output filename by replacing the source extension 
static void build_filename(const char *src, const char *ext, char *buf, size_t bufsz) {
    const char *dot  = strrchr(src, '.');
    size_t base_len  = dot ? (size_t)(dot - src) : strlen(src);
    if (base_len > bufsz - 6) base_len = bufsz - 6;
    memcpy(buf, src, base_len);
    buf[base_len] = '\0';
    strncat(buf, ext, bufsz - base_len - 1);
}

/* do_pass — the single routine used for BOTH passes 
  pass 1: scan, build label table, no code emitted, no error on undef labels
  pass 2: encode instructions into obj_code[], error on undefined labels
*/
static void do_pass(FILE *fp, int pass) {
    char  line[MAX_LINE];
    int   lineno = 0;
    int   pc = 0;

    rewind(fp); // reads file pointer to the start of the file
    if (pass == 2) obj_count = 0; // reset object code

    while (fgets(line, sizeof line, fp)) { 
        lineno++;
        line[strcspn(line, "\r\n")] = '\0';
        const char *p = skip(line);
        if (*p == '\0' || *p == ';') continue; 

        // Label detection 
        char label_name[MAX_TOKEN] = {0};
        bool has_label = false;

        const char *q = p;
        while (*q && *q != ':' && *q != ' ' && *q != '\t' && *q != ';')
            q++;

        if (*q == ':') {
            int len = (int)(q - p);
            if (len <= 0 || len >= MAX_TOKEN) { // checks the length of label name
                report_error(lineno, "label name empty or too long");
            } else {
                snprintf(label_name, MAX_TOKEN, "%.*s", len, p);

                if (!valid_label_name(label_name)) { // if label name is invalid
                    char buf[MAX_LINE];
                    snprintf(buf, sizeof buf, "invalid label name '%s'", label_name);
                    report_error(lineno, buf);
                } else {
                    has_label = true;
                    if (pass == 1 && !label_add(label_name, pc)) {
                        char buf[MAX_LINE]; // temporary string buffer stores formatted error msgs 
                        snprintf(buf, sizeof buf, "duplicate label '%s'", label_name); 
                        report_error(lineno, buf);
                    }
                }
            }
            p = q + 1;  // advance past ':'
        }

        p = skip(p);
        if (*p == '\0' || *p == ';') continue;  // label-only line

        // Parse mnemonic 
        // Extracts the instruction mnemonic from the line
        char mnem[MAX_TOKEN] = {0};
        {
            int i = 0;
            while (*p && !isspace((unsigned char)*p) && *p != ';'
                   && i < MAX_TOKEN - 1)
                mnem[i++] = *p++;
        }

        p = skip(p);

        // Parse operand 
        // Extracts operand from the line following the mnemonic
        char opstr[MAX_LINE] = {0};
        bool has_op = false;

        if (*p && *p != ';') {
            int i = 0;
            while (*p && !isspace((unsigned char)*p) && *p != ';' && *p != ',' && i < (int)sizeof opstr - 1)
                opstr[i++] = *p++;
            has_op = true;

            // Anything after the operand (before ';') is an error
            p = skip(p);
            if (*p && *p != ';') {
                char buf[MAX_LINE];
                snprintf(buf, sizeof buf, "unexpected text after operand: '%s'", p);
                report_error(lineno, buf);
            }
        }

        // Instruction lookup 
        const Instr *id = instr_find(mnem);
        if (!id) {
            char buf[MAX_LINE];
            snprintf(buf, sizeof buf, "unknown mnemonic '%s'", mnem);
            report_error(lineno, buf);
            continue;
        }

        // Validate operand presence 
        if (id->has_operand && !has_op) {
            char buf[MAX_LINE];
            snprintf(buf, sizeof buf, "'%s' requires an operand", mnem);
            report_error(lineno, buf);
            continue;
        }
        if (!id->has_operand && has_op) {
            char buf[MAX_LINE];
            snprintf(buf, sizeof buf, "'%.32s' takes no operand (found '%.256s')", mnem, opstr);
            report_error(lineno, buf);
            // still assemble the instruction, operand is ignored
        }

        // Pseudo-instructions
        // data <value>  — reserve and initialise one word , memory word generated
        if (strcmp(id->mnemonic, "data") == 0) {
            int val = 0;
            if (has_op) parse_operand(opstr, pc, false, pass, lineno, &val);
            if (pass == 2) emit_word((int32_t)val);
            pc++;
            continue;
        }

        // SET <value>  — assign constant to the label on this line
        if (strcmp(id->mnemonic, "SET") == 0) {
            int val = 0;
            if (has_op) parse_operand(opstr, pc, false, pass, lineno, &val);
            if (pass == 1 && has_label) {
                int idx = label_find(label_name);
                if (idx >= 0) label_table[idx].value = val;
            }
            continue;  
        }

        // Real instruction
        int operand = 0;
        if (id->has_operand && has_op)
            parse_operand(opstr, pc, id->is_branch, pass, lineno, &operand);

        if (pass == 2)
            emit_word(encode_word(id->opcode, operand));

        pc++;
    }
}

// Write binary object file 
// Format: consecutive 32-bit little-endian integers, one per assembled word.
static bool write_object(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Cannot create object file '%s': %s\n",
                filename, strerror(errno));
        return false;
    }

    for (int i = 0; i < obj_count; i++) {
        uint32_t w = (uint32_t)obj_code[i];
        uint8_t  b[4] = { // convert word to little-endian bytes
            (uint8_t)( w & 0xFF),
            (uint8_t)((w >>  8) & 0xFF),
            (uint8_t)((w >> 16) & 0xFF),
            (uint8_t)((w >> 24) & 0xFF)
        };
        fwrite(b, 1, 4, fp); // fwrite() = writes binary data from memory into a file
    }

    fclose(fp);
    return true;
}

/* Write advanced listing file 
 Each line with code:  <8-hex-addr> <8-hex-word>  <mnemonic> [<operand>]
 Label-definition line: <8-hex-addr> <label>:
 Unused-label warnings printed at the bottom.
*/
static bool write_listing(const char *filename, FILE *src) {
    FILE *lst = fopen(filename, "w");
    if (!lst) {
        fprintf(stderr, "Cannot create listing file '%s': %s\n", filename, strerror(errno));
        return false;
    }

    char line[MAX_LINE];
    int  lineno = 0, pc = 0, oi = 0; // oi = index to obj_code[]

    rewind(src);

    while (fgets(line, sizeof line, src)) {
        lineno++;
        line[strcspn(line, "\r\n")] = '\0';

        const char *p = skip(line);
        if (*p == '\0' || *p == ';') continue;

        // Check for label 
        char label_name[MAX_TOKEN] = {0};
        bool has_label = false;

        const char *q = p;
        while (*q && *q != ':' && *q != ' ' && *q != '\t' && *q != ';') q++;
        if (*q == ':') {
            int len = (int)(q - p);
            if (len > 0 && len < MAX_TOKEN) { // checks the length of the label name
                snprintf(label_name, MAX_TOKEN, "%.*s", len, p);
                if (valid_label_name(label_name)) { // checks for a valid label name
                    has_label = true;
                    fprintf(lst, "%08X %s:\n", pc, label_name);
                }
            }
            p = q + 1;
        }

        p = skip(p);
        if (*p == '\0' || *p == ';') continue;

        // Mnemonic 
        char mnem[MAX_TOKEN] = {0};
        {
            int i = 0;
            while (*p && !isspace((unsigned char)*p) && *p != ';'
                   && i < MAX_TOKEN - 1)
                mnem[i++] = *p++;
        }
        p = skip(p);

        // Operand 
        char opstr[MAX_LINE] = {0};
        bool has_op = false;
        if (*p && *p != ';') {
            int i = 0;
            while (*p && !isspace((unsigned char)*p) && *p != ';' && *p != ',' && i < (int)sizeof opstr - 1)
                opstr[i++] = *p++;
            has_op = true;
        }

        // Lookup instruction
        const Instr *id = instr_find(mnem);
        if (!id) continue;
        if (strcmp(id->mnemonic, "SET") == 0) continue;  // no code

        // Resolve operand value 
        int operand = 0;
        if (id->has_operand && has_op)
            parse_operand(opstr, pc, id->is_branch, 2, lineno, &operand);

        // Format operand for listing 
        char op_display[MAX_TOKEN] = {0};
        if (id->has_operand) {
            if (id->is_branch) {
                int target = pc + 1 + operand;
                int li = label_at_addr(target);
                if (li >= 0)
                    snprintf(op_display, sizeof op_display, "%s", label_table[li].name);
                else
                    snprintf(op_display, sizeof op_display, "%d", operand);
            } else {
                // For non-branch, show label name if operand came from one
                int li = label_at_addr(operand);
                bool from_label = (has_op && !isdigit((unsigned char)opstr[0]) && opstr[0] != '+' && opstr[0] != '-');
                if (li >= 0 && from_label)
                    snprintf(op_display, sizeof op_display, "%s", label_table[li].name);
                else
                    snprintf(op_display, sizeof op_display, "%d", operand);
            }
        }

        // Print listing line 
        /*if (strcmp(id->mnemonic, "data") == 0) {
            fprintf(lst, "%08X %08X  data %d\n", pc, (uint32_t)obj_code[oi], operand);
        } else if (id->has_operand) {
            fprintf(lst, "%08X %08X  %s %s\n", pc, (uint32_t)obj_code[oi], id->mnemonic, op_display);
        } else {
            fprintf(lst, "%08X %08X  %s\n", pc, (uint32_t)obj_code[oi], id->mnemonic);
        }
        */
       /*if (id == ':') {
            int len = (int)(q - p);
            if (len > 0 && len < MAX_TOKEN) { // checks the length of the label name
                snprintf(label_name, MAX_TOKEN, "%.*s", len, p);
                if (valid_label_name(label_name)) { // checks for a valid label name
                    has_label = true;
                    fprintf(lst, "%08X %s:\n", pc, label_name);
                }
            }
            p = q + 1;
        }
            */
        
        if (id->has_operand) {
            fprintf(lst, "%08X %s\n",pc,id->mnemonic);
        } else {
            fprintf(lst,"%08X %s\n",pc,id->mnemonic);
        }

        oi++;
        pc++;
        (void)has_label;
    }

    // Unused-label warnings at the bottom of the listing 
    for (int i = 0; i < label_count; i++)
        if (!label_table[i].used)
            fprintf(lst, "; WARNING: label '%s' defined but never used\n", label_table[i].name);

    fclose(lst);
    return true;
}

// Warn about unused labels 
static void check_unused_labels(void) {
    for (int i = 0; i < label_count; i++) {
        if (!label_table[i].used) {
            char buf[MAX_LINE];
            snprintf(buf, sizeof buf, "label '%s' defined but never used", label_table[i].name);
            report_warning(0, buf);
        }
    }
}

// main 
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source.asm>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *src = fopen(argv[1], "r");
    if (!src) {
        fprintf(stderr, "Cannot open '%s': %s\n", argv[1], strerror(errno));
        return EXIT_FAILURE;
    }

    // creates 3 files .o , .lst and .log
    char obj_name[MAX_LINE], lst_name[MAX_LINE], log_name[MAX_LINE];
    build_filename(argv[1], ".o",   obj_name, sizeof obj_name);
    build_filename(argv[1], ".lst", lst_name, sizeof lst_name);
    build_filename(argv[1], ".log", log_name, sizeof log_name);

    log_fp = fopen(log_name, "w");
    if (!log_fp)
        fprintf(stderr, "Warning: cannot create log file '%s'\n", log_name);

    // Pass 1: build label table (label scan)
    do_pass(src, 1);

    // Pass 2: assemble and generates machine code
    do_pass(src, 2);

    // Check and warn about unused labels 
    check_unused_labels();

    // Write output files 
    if (error_count == 0) {
        if (write_object(obj_name))
            printf("Object  : %s  (%d words)\n", obj_name, obj_count);
        if (write_listing(lst_name, src))
            printf("Listing : %s\n", lst_name);
    } else {
        printf("Assembly failed -  no output files produced.\n");
    }

    printf("Result  : %d error(s), %d warning(s)\n", error_count, warning_count);

    if (log_fp) {
        fprintf(log_fp, "\n--- %d error(s), %d warning(s) ---\n", error_count, warning_count);
        fclose(log_fp);
    }

    fclose(src);
    return error_count > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}