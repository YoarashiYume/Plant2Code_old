#pragma once
#define MAKE_STR_(s) #s
#define PATH_SEPARATOR '\\'

#define REF_ENTER_POINT "Вход"
#define STRUCT_DATA "struct.csv"
#define CSV_SEPARATOR ';'
#define CSV_BORDER '\"'
#define CSV_STRUCT_NAME "Обозначение"
#define CSV_STRUCT_TYPE "Тип данных"

#define CSV_SINGALS_NAME_VAR_1 "Обозначение сигнала"
#define CSV_SINGALS_NAME_VAR_2 "Обозначение состояния"
#define CSV_SINGALS_NAME_VAR_3 "Обозначение"
#define CSV_SINGALS_TYPE CSV_STRUCT_TYPE
#define CSV_CONST_VALUE "Значение"


#define CSV_SINGAL_COMMENT_1 "Наименование"
#define CSV_SINGAL_COMMENT_2 "Название состояния"
#define CSV_SINGAL_COMMENT_3 "Название сигнала"

#define CSV_SINGAL_COMMENT {CSV_SINGAL_COMMENT_1, CSV_SINGAL_COMMENT_2, CSV_SINGAL_COMMENT_3}

#define CSV_UNUSED_SIGNAL "Резерв"

#define INNER_SIGNAL 'Z'
#define INPUT_SIGNAL 'X'
#define OUTPUT_SIGNAL 'Y'
#define TIMER_SIGNAL 'T'
#define TIMER_TYPE "UInt32"
#define CONST_SIGNAL 'C'
#define STRUCT_SIGNAL 'S'

#define STRUCT_FIELD_SYMBOL '.'

#define FLOAT_SIGN '.'

#define EXPANSION_SYMBOL "..."
#define ARRAY_SEPAROTOR ".."

#define ASSIGN_SIGN '='
#define ASSIGN_LINE_SIGN "="
#define SPACE_SIGN ' '

#define MINUS_SYMBOL "-"


#define MAX_UNSPIN_ITERATION 1000

#define INCORRENT_BRACKET_LINE ") )"

#define PTR_SIGN '*'
#define LEFT_SQARE_BRACKET '['
#define RIGHT_SQARE_BRACKET ']'

#define LEFT_SIMPLE_BRACKET '('
#define RIGHT_SIMPLE_BRACKET ')'

#define STRING_TOPIC '\"'

#define INCORRECT_FLOAT_CHAR ','
#define CORRECT_FLOAT_CHAR '.'

#define PLANTUML_MIN_REF_SIZE 3
#define PLANTUML_FUNCTION_WRAP '"'
#define PLANTUML_FUNCTION_ARGS_SEPARATOR ','
#define PLANTUML_STRUCT_ACCESS_SIGN '.'
#define PLANTUML_SETTING_SIGN1 '!'
#define PLANTUML_SETTING_SIGN2 '@'
#define PLANTUML_SINGLE_COMMENT '\''
#define PLANTUML_BLOCK_COMMENT_OPEN "/'"
#define PLANTUML_BLOCK_COMMENT_CLOSE "'/"
#define PLANTUML_CODE_COMMENT "//"
#define PLANTUML_CODE_COMMENT_START "*\\"
#define PLANTUML_CODE_COMMENT_END "/*"
#define PLANTUML_CODE_COMMENT_START_ "$comment_start"
#define PLANTUML_CODE_COMMENT_END_ "$comment_end"

#define PLANTUML_NODE_NEW_LINE "%newline()"

#define PLANTUML_START_COMMAND_BLOCK ':'
#define SERVICE_SYMBOLS {'@', '#', '$'}
#define PLANTUML_DIAG_END_WORD "@enduml"
#define PUML_FUNCTION_END '|'
#define PUML_REF_END ';'
#define PUML_SINGNATURE_ARGS_SPLIT "->"
#define PUML_CALCULATION_END ']'
#define TERMINATE_SYMBOLS {PUML_FUNCTION_END, PUML_REF_END, PUML_CALCULATION_END}

#define PLANTUML_LINE_SPLITTER ' '


#define PLANTUML_REF_WORD "Вход"
#define PLANTUML_REF_OUT_WORD "Выход"
#define PLANTUML_NOTE_START "note"
#define PLANTUML_NOTE_START_WhERE {"right", "left", "up", "down"}
#define PLANTUML_NOTE_END "end note"
#define PLANTUML_NODE_NAME_SIMPLE_CH ':'

#define PLANTUML_PAR_MARKER ':'
#define PLANTUML_INPUT_PAR "Параметры"
#define PLANTUML_OUTPUT_PAR "Возвращаемые значения"
#define PLANTUML_LOCAL_PAR "Локальные переменные"
#define PLANTUML_DETALED_PAR "Примечание"
#define PLANTUML_DETALED_PAR_2 "Примечания"

#define PLANTUML_INPUT_PAR_CH 'P'
#define _PLANTUML_OUTPUT_PAR_CH 'R'
#define PLANTUML_LOCAL_PAR_CH 'L'

#define PLANTUML_THEN "then"
#define PLANTUML_IS "is"
#define PLANTUML_DETACH "detach"

#define PLANTUML_IF "if"
#define PLANTUML_ELSE "else"
#define PLANTUML_ELSE_IF_SEP "else if"
#define PLANTUML_ELSE_IF "elseif"
#define PLANTUML_WHILE "while"
#define PLANTUML_SWITCH "switch"
#define PLANTUML_CASE "case"
#define PLANTUML_END_WORD "end"
#define PLANTUML_END_WORD_IF "endif"
#define PLANTUML_END_WORD_WHILE "endwhile"
#define PLANTUML_END_WORD_SWITCH "endswitch"
#define PLANTUML_START_KEY_WORDS {PLANTUML_IF,PLANTUML_WHILE,PLANTUML_SWITCH }

#define PTR_CHAR '*'
#define CASE_WORD_COUNT 1


#define DOXYGEN_BRIEF "\\brief "
#define DOXYGEN_PARAM_IN "\\param [in] "
#define DOXYGEN_PARAM_OUT "\\param [out] "
#define DOXYGEN_PARAM_IN_OUT "\\param [in,out] "
#define DOXYGEN_RETURN "\\return "


#define STRUCT_LIB "transferredStruct"
#define STRUCT_FOR_LIB "TransferredStruct"
#define STRUCT_FOR_LIB_NAME "signalName"
#define STRUCT_FOR_LIB_TYPE "signalType"
#define STRUCT_FOR_LIB_PTR "ptr"
#define STRUCT_FOR_LIB_IS_PTR "isPtr"
#define STRUCT_FOR_LIB_ELEMENT_COUNT "elementCount"
#define STRUCT_FOR_LIB_SIGNAL_GROUPE "signalGroupe"

#define LIB_SIGNAL_GET "getSignalData"
#define LIB_SIGNAL_CLEAR "clearSignalData"
#define LIB_MAIN_FUNCTION "oneStepFunction"
#define LIB_SIGNAL_RESET "signalReset"
#define LIB_INPUT_SIGNAL_RESET "signalResetInput"

#define LIB_SIGNAL_GET_SIZE "size"


#define TERM_ABS_START '#'
#define TERM_ABS_END '@'
#define TERM_ABS '|'
#define TERM_ABS_STR "|"

#define TERM_ABS_START_STR "#"
#define TERM_ABS_END_STR "@"
#define TERM_ARRAY_SIGN_STR "$"

#define TERM_ARRAY_SIGN '$'
#define TERM_ARRAY_STR "$"
#define TERM_ARRAY_HOLDER " $ "
#define TERM_TERNAR_START "?"
#define TERM_TERNAR_SEPARATOR ":"
#define TERM_ARRAY_INC "++"
#define TERM_ARRAY_DEC "--"
#define TERM_LEFT_SIMPLE_BRACKET "("
#define TERM_RIGHT_SIMPLE_BRACKET ")"
#define TERM_POW "^"


#define SOURCE_FILE "SourceInfo."
#define HEADER_FILE "HeaderInfo."
#define LIB_FILE "LibInfo."

#define GEN_INPUT "Входной"
#define GEN_INNER "Внутренний"
#define GEN_OUTPUT "Выходной"

#define GEN_STRUCT_FIELD_MARKER "_"
#define GEN_DIV '/'

#define GEN_ACCESS_PREFIX "getSignal"

#define LANGUAGE_YES "да"
#define LANGUAGE_NO "нет"