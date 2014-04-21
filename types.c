/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Description:                                                                *
 *                                                                             *
 * Written by Vu Quang Hoa for COMP9021                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Number of keywords
#define NB_OF_KEYWORDS 18
//Maximum number of sentences = 100
#define MAX 100

//An array of all keywords which can appear in INPUT
const char *c_keywords[MAX] = {"int", "char", "long", "short", "double", "float", "signed", "unsigned"
    ,"array", "of", "to", "void", "datum", "type", "returning", "function", "pointer", "data"};

/* sentences[][0] -> where the sentence begin
 * sentences[][1] -> basic types(int, short...) 1/array 2/pointer 3/function 4/ mixed 5
 * sentences[][2] -> 
 * -1 if No reference to other variables
 * i if reference to variable defined in i sentence
 */
int sentences[MAX][3];

/* Store sentences string
 * which is used to print out result
 */
char *sentences_string[MAX];

//Number of sentences in INPUT
int NB_OF_SENTENCES;

/* variable[][0] -> "" if NULL
 * or defined variable
 * variable[][1] -> "" if NULL
 * or i - the variable refer to other variable in sentence i
 */
char *variable[MAX][2];

/* flag[index], index is
 * 0 - signed
 * 1 - unsigned
 * 2 - int
 * 3 - short
 * 4 - double
 * 5 - float
 * 6 - char 
 * 7 - long
 * 8 - long long
 */
int flag[MAX][10] = {{0}};
//The number of flag
#define NB_OF_FLAG 9
//An array stores all string of flag to print out BASIC TYPE DESCRIPTION
char flag_string[][10] = {"signed", "unsigned", "int", "short", "double", "float", "char", "long", "long long"};

//Check array is used to check again all sentences which are correct or not
bool check_variable[MAX];
/* An array store 1 or 0
 * If this sentence is referenced by other -> 1, otherwise is 0
 */
bool array_variable[MAX];

//Checking functions
bool check_name_of_argument(char *);
bool check_key_word(char *);
bool check_basic_type_description (int, int, char **);
bool check_vowel(char);
bool get_variable(char *, int);
bool check_number(char *, int);

//Construct sentence and print output functions
void construct_sentence(int, int, char **);
//Print out the result if INPUT is correct
void print_result(int, char **);

//Processing the INPUT, return FALSE if INPUT is incorrect and TRUE otherwise
bool input_processing(int, int, char **);

//First level processing functions
bool first_level_processing(int, int, char **);

//Second and third level of processing functions
bool high_level_processing(int, int, char **);
bool get_pointer_phrase(int, int, char **);
bool get_array_phrase(int, int, char **);
bool get_function_phrase(int, int, char **);

//Fourth level of processing functions
bool assigning_variable_fourth_level();
void travel_sentences(int);
void string_combining(char *, int, char *, int);

int main(int argc, char **argv) {
    //End of a phrase or not
    bool end_a_phrase = false;
    int start_index = 0, end_index = 0;
    //Count the number of phrase
    int count_phrase = 0;
    //Check variable -> check input
    bool check = true;
    /* Check all arguments, stop sentence if we meet character '.' */
    for (int i = 1; i < argc; i++) {
        end_a_phrase = false;
        //If this argument is the last argument of phrase -> pin end_index.
        //start_index = end_index + 1
        if (argv[i][strlen(argv[i]) - 1] == '.') {
            *(*(argv + i) + strlen(argv[i]) - 1) = '\0';
            count_phrase++;
            end_a_phrase = true;
            start_index = end_index + 1;
            end_index = i;
        }
        if (!end_a_phrase)
            continue;
        //Processing the input
        if (!input_processing(end_index - start_index, count_phrase, argv + start_index)) {
            printf("Incorrect input\n");
            return EXIT_FAILURE;
        }
    }
    if (end_index != argc - 1)
        check = false;
    //Assign to number of sentences NB_OF_SENTENCES
    NB_OF_SENTENCES = count_phrase;
    if (check) {
        for (int i = 1; i <= NB_OF_SENTENCES; i++) {
            check_variable[i] = false;
        }
        if (assigning_variable_fourth_level())
            print_result(argc - 1, argv);
        else
            printf("Incorrect input\n");
    }
    else
        printf("Incorrect input\n");
    //printf("\nCount-phrase : %d", count_phrase);
    return EXIT_SUCCESS;
}

//Function input_processing : process input, find out which type of INPUT
//Return TRUE if INPUT is correct and FALSE otherwise
bool input_processing(int offset, int count_sentences, char **argv) {
    //Assign to the sentence array
    //Starting argument of sentence
    
    sentences[count_sentences][0] = offset;
    //Which type of the sentence
    /* If this sentence contains "array" -> sentences[count_sentences][1] = 2
     * If this sentence contains "pointer" -> sentences[count_sentences][1] = 3
     * If this sentence contains "function" -> sentences[count_sentences][1] = 4
     */
    sentences[count_sentences][1] = 1;
    for (int i = 1; i <= offset; i++) {
        if (!strcmp(argv[i], "array")) {
            sentences[count_sentences][1] = 2;
            break;
        }
        else if (!strcmp(argv[i], "pointer")) {
            sentences[count_sentences][1] = 3;
            break;
        }
        else if (!strcmp(argv[i], "function")) {
            sentences[count_sentences][1] = 4;
            break;
        }
    }
    int nb_of_string_type = 0;
    for (int i = 0; i <= offset; i++) {
        if (!strcmp(argv[i], "type"))
            nb_of_string_type++;
    }
    int nb_of_string_array_pointer_function = 0;
    for (int i = 0; i <= offset; i++) {
        if (!strcmp(argv[i], "array") || !strcmp(argv[i], "arrays"))
            nb_of_string_array_pointer_function++;
        if (!strcmp(argv[i], "pointer") || !strcmp(argv[i], "pointers"))
            nb_of_string_array_pointer_function++;
        if (!strcmp(argv[i], "function"))
            nb_of_string_array_pointer_function++;
    }
    //If this sentence contains "array" and "pointer" and "function" -> sentences[count_sentences][1] = 5
    //If this sentence contains 2 string "type" -> sentences[count_sentences][1] = 5
    if (nb_of_string_type >= 2 || nb_of_string_array_pointer_function >= 2)
        sentences[count_sentences][1] = 5;

    //Set this sentence references to NONE
    sentences[count_sentences][2] = -1;//NONE
    sentences_string[count_sentences] = malloc(sizeof(char) * 100);
    if (sentences[count_sentences][1] == 1)
        return first_level_processing(offset, count_sentences, argv);
    else if (sentences[count_sentences][1] == 2) {
        //If the sentence is ARRAY sentence
        if (strcmp(argv[2], "of")) {
            if (!get_variable(argv[2], count_sentences))
                return false;
            strcat(sentences_string[count_sentences], argv[2]);
            return get_array_phrase(offset - 3, count_sentences, argv + 3);
        }
        return get_array_phrase(offset - 2, count_sentences, argv + 2);
    }
    else if (sentences[count_sentences][1] == 3) {
        //If the sentence is POINTER sentence
        if (strcmp(argv[2], "to")) {
            if (!get_variable(argv[2], count_sentences))
                return false;
            strcat(sentences_string[count_sentences], "*");
            strcat(sentences_string[count_sentences], argv[2]);
            return get_pointer_phrase(offset - 3, count_sentences, argv + 3);
        }
        strcat(sentences_string[count_sentences], "*");
        return get_pointer_phrase(offset - 2, count_sentences, argv + 2);
    }
    else if (sentences[count_sentences][1] == 4) {
        //If the sentence is FUNCTION sentence
        if (!strcmp(argv[2], "returning")) {
            return get_function_phrase(offset - 2, count_sentences, argv + 2);
        }
        else {
            if (!strcmp(argv[2], "a") || !strcmp(argv[2], "A") || !strcmp(argv[2], "An"))
                return false;
            if (!check_key_word(argv[2]) && check_name_of_argument(argv[2]))
                variable[count_sentences][0] = argv[2];
            else
                return false;
            strcat(sentences_string[count_sentences], variable[count_sentences][0]);
            return get_function_phrase(offset - 3, count_sentences, argv + 3);
        }
    }
    else if (sentences[count_sentences][1] == 5) {
        //If the sentence is MIXED sentence
        return high_level_processing(offset, count_sentences, argv);
    }
    return false;
}

//Print out result of all sentences.
void print_result(int argc, char **argv) {
    sentences[NB_OF_SENTENCES + 1][0] = argc;
    for (int i = 1; i <= NB_OF_SENTENCES; i++) {
        printf("%s\n", sentences_string[i]);
    }
}

//Construct sentence based on flag array (add TYPE to sentence)
void construct_sentence(int offset, int count_sentences, char **argv) {
    if ((flag[count_sentences][8] || flag[count_sentences][7]) && flag[count_sentences][3]) {
        flag[count_sentences][3] = 0;
    }
    /* If TYPE of this sentence is "signed char" -> add to sentences_string */
    if (flag[count_sentences][0] && flag[count_sentences][6]) {
        strcat(sentences_string[count_sentences], flag_string[0]);
        strcat(sentences_string[count_sentences], " ");
    }
    /* If TYPE of this sentence contains "unsigned" -> add to sentences_string */
    if (flag[count_sentences][1]) {
        strcat(sentences_string[count_sentences], flag_string[1]);
        strcat(sentences_string[count_sentences], " ");
    }
    /* If TYPE of this sentence contains "long" or "long long" -> add to sentences_string */
    if (flag[count_sentences][7]) {
        strcat(sentences_string[count_sentences], flag_string[7]);
        strcat(sentences_string[count_sentences], " ");
    }
    else if (flag[count_sentences][8]) {
        strcat(sentences_string[count_sentences], flag_string[8]);
        strcat(sentences_string[count_sentences], " ");
    }
    /* If TYPE of this sentence has type ("int", "float"....)-> add to sentences_string */
    for (int i = 2; i < NB_OF_FLAG - 2; i++) {
        if (flag[count_sentences][i]) {
            if (i == 2 && flag[count_sentences][i] && flag[count_sentences][1])
                continue;
            strcat(sentences_string[count_sentences], flag_string[i]);
            strcat(sentences_string[count_sentences], " ");
        }
    }
}

//Check name of argument, TRUE if satisfied, FALSE otherwise
bool check_name_of_argument(char *string) {
    if (string[0] != '_' && !isalpha((int)string[0]))
        return false;
    for (int i = 1; i < strlen(string); i++) {
        if (string[i] != '_' && !isalpha((int)string[i]) && !(string[i] <= '9' && string[i] >= '0')) 
            return false;
    }
    return true;
}

//If this string is a keyword -> TRUE
//If not -> FALSE
bool check_key_word(char *string) {
    for (int i = 0; i < NB_OF_KEYWORDS; i++) {
        if (!strcmp(string, c_keywords[i])) 
            return true;
    }
    return false;
}

//Check character is vowel or not, return TRUE if vowel
bool check_vowel(char c) {
    if (c == 'a' || c == 'u' || c == 'o' || c == 'e' || c == 'i')
        return true;
    return false;
}

//Check string contains all digits or not
bool check_number(char *string, int length) {
    if (string[0] == '0')
        return false;
    for (int i = 0; i < length; i++) {
        if (!isdigit((int)string[i]))
            return false;
    }
    return true;
}

//Check "BASIC TYPE DESCRIPTION" of sentence
bool check_basic_type_description (int offset, int count_sentences, char **argv) {
    bool check_type = false;
    //Assigning type, sign, long of sentence
    for (int i = 0; i <= offset; i++) {
        //Assigning the sign in the sentence : "signed" or "unsigned"
        if (!strcmp(argv[i], "signed") && !flag[count_sentences][0] && !flag[count_sentences][1])
            flag[count_sentences][0] = 1;
        else if (!strcmp(argv[i], "signed") && (flag[count_sentences][0] || flag[count_sentences][1]))
            return false;
        if (!strcmp(argv[i], "unsigned") && !flag[count_sentences][0] && !flag[count_sentences][1])
            flag[count_sentences][1] = 1;
        else if (!strcmp(argv[i], "unsigned") && (flag[count_sentences][0] || flag[count_sentences][1]))
            return false;
        
        //Assigning the number of "long" in the sentence
        if (!strcmp(argv[i], "long") && !flag[count_sentences][7] && !flag[count_sentences][8]) {
            flag[count_sentences][7] = 1;
        }
        else if (!strcmp(argv[i], "long") && flag[count_sentences][7]) {
            flag[count_sentences][7] = 0;
            flag[count_sentences][8] = 1;
        }
        else if (!strcmp(argv[i], "long") && !flag[count_sentences][7] && flag[count_sentences][8]) {
            return false;
        }
        //Check type int
        if (!strcmp(argv[i], "int") && !check_type) {
            check_type = true;
            flag[count_sentences][2] = 1;
        }
        else if (!strcmp(argv[i], "int") && check_type && flag[count_sentences][3] && !flag[count_sentences][2]) {
            flag[count_sentences][2] = 1;
        }
        else if (!strcmp(argv[i], "int") && check_type && flag[count_sentences][3] && flag[count_sentences][2]) {
            return false;
        }
        else if (!strcmp(argv[i], "int") && check_type && !flag[count_sentences][3]) {
            return false;
        }
        //Check type short
        if (!strcmp(argv[i], "short") && !check_type) {
            check_type = true;
            flag[count_sentences][3] = 1;
        }
        else if (!strcmp(argv[i], "short") && check_type && !flag[count_sentences][3] && flag[count_sentences][2]) {
            flag[count_sentences][3] = 1;
        }
        else if (!strcmp(argv[i], "short") && check_type && flag[count_sentences][3] && flag[count_sentences][2]) {
            return false;
        }
        else if (!strcmp(argv[i], "short") && check_type && !flag[count_sentences][2]) {
            return false;
        }
        //Check type double
        if (!strcmp(argv[i], "double") && !check_type) {
            check_type = true;
            flag[count_sentences][4] = 1;
        }
        else if (!strcmp(argv[i], "double") && check_type) {
            return false;
        }
        
        //Check type float
        if (!strcmp(argv[i], "float") && !check_type) {
            check_type = true;
            flag[count_sentences][5] = 1;
        }
        else if (!strcmp(argv[i], "float") && check_type) {
            return false;
        }
        //Check type char
        if (!strcmp(argv[i], "char") && !check_type) {
            check_type = true;
            flag[count_sentences][6] = 1;
        }
        else if (!strcmp(argv[i], "char") && check_type) {
            return false;
        }
    }
    
    /* Check all types, signs and long in the sentence
     * We did not accept 
     * both signed an unsigned
     * both long and long long
     * 2types at the same time except short and int
     */
    if (flag[count_sentences][0] && flag[count_sentences][1])
        return false;
    if (flag[count_sentences][7] && flag[count_sentences][8])
        return false;
    
    if (flag[count_sentences][2] && flag[count_sentences][3]) {
        flag[count_sentences][2] = 0;
    }
    
    //If the type is float, we cannot assign sign.
    if (flag[count_sentences][5] && (flag[count_sentences][0] || flag[count_sentences][1]))
        return false;
    
    /* If type is not set -> assign default type is INT with BASIC TYPE DESCRIPTION
     * If sign is not set -> assign default type is signed
     * If we cannot find the type in this sentence -> WRONG input
     */
    if (!check_type && sentences[count_sentences][1] == 1) {
        if (flag[count_sentences][0] || flag[count_sentences][1] || flag[count_sentences][7] || flag[count_sentences][8]) {
            flag[count_sentences][2] = 1;
            check_type = true;
        }
        else
            return false;
    }
    else if (!check_type && sentences[count_sentences][1] != 1)
        return false;
    //If this sentence contains "long long" and "int" -> default presentation is "long long"
    if (flag[count_sentences][8] && flag[count_sentences][2])
        flag[count_sentences][2] = 0;
    //If this sentence contains "long" and "int" -> default presentation is "long"
    if (flag[count_sentences][7] && flag[count_sentences][2])
        flag[count_sentences][2] = 0;
    //If the sentence contains both "long long" and "double" -> WRONG input
    if (flag[count_sentences][8] && flag[count_sentences][4])
        return false;
    //If the sentence contains both "long long" or "long" and "float" -> WRONG input
    if (flag[count_sentences][7] && flag[count_sentences][5])
        return false;
    if (flag[count_sentences][8] && flag[count_sentences][5])
        return false;
    
    //Find variable in the sentence
    if (!check_key_word(argv[offset]) && !check_name_of_argument(argv[offset]))
        return false;
    else if (check_name_of_argument(argv[offset]) && !check_key_word(argv[offset])) {
        if (!strcmp(argv[offset], "a") || !strcmp(argv[offset], "A") || !strcmp(argv[offset], "An"))
            return false;
        variable[count_sentences][0] = argv[offset];
    }
    
    return true;
}

//Function first_level_processing: primarily process sentences
//If a sentence is BASIC TYPE sentence -> check and print out if satisfied
bool first_level_processing(int offset, int count_sentences, char **argv) {
    //Check first argument of the sentence is "A" or "An"
    if (strcmp(argv[0], "A") && strcmp(argv[0], "An"))
        return false;
    if (!strcmp(argv[0], "A")) {
        if (check_vowel(argv[1][0]))
            return false;
    }
    else {
        if (!check_vowel(argv[1][0]))
            return false;
    }
    //Construct the sentence string -> assign to sentences_string array
    sentences_string[count_sentences] = malloc(sizeof(char) * 100);
    if (!check_basic_type_description(offset - 1, count_sentences, argv + 1))
        return false;
    if (sentences[count_sentences][1] == 1) {
        construct_sentence(sentences[count_sentences + 1][0] - sentences[count_sentences][0] - 1, count_sentences, argv + sentences[count_sentences][0]);
        if (variable[count_sentences][0])
            strcat(sentences_string[count_sentences], variable[count_sentences][0]);
    }
    return true;
}

//High level sentences processing
//Check the sentence is ARRAY, POINTER, FUNCTION -> Assign to appropriate function
bool high_level_processing(int offset, int count_sentences, char **argv) {
    sentences_string[count_sentences] = malloc(sizeof(char) * 100);
    
    //If this sentence is ARRAY sentence -> assign to function get_array_phrase
    if (!strcmp(argv[0], "An") && !strcmp(argv[1], "array")) {
        if (!strcmp(argv[2], "of")) {
            return get_array_phrase(offset - 2, count_sentences, argv + 2);
        }
        else {
            if (!get_variable(argv[2], count_sentences))
                return false;
            strcat(sentences_string[count_sentences], variable[count_sentences][0]);
            return get_array_phrase(offset - 3, count_sentences, argv + 3);
        }
    }
    //If this sentence is POINTER sentence -> assign to function get_pointer_phrase
    else if (!strcmp(argv[0], "A") && !strcmp(argv[1], "pointer")) {
        
        if (!strcmp(argv[2], "to")) {
            strcat(sentences_string[count_sentences], "*");
            return get_pointer_phrase(offset - 2, count_sentences, argv + 2);
        }
        else {
            if (!get_variable(argv[2], count_sentences))
                return false;
            strcat(sentences_string[count_sentences], "*");
            strcat(sentences_string[count_sentences], variable[count_sentences][0]);
            //strcat(sentences_string[count_sentences], variable[count_sentences][0]);
            return get_pointer_phrase(offset - 3, count_sentences, argv + 3);
        }
    }
    //If this sentence is FUNCTION sentence -> assign to function get_function_phrase
    else if (!strcmp(argv[0], "A") && !strcmp(argv[1], "function")) {
        if (!strcmp(argv[2], "returning"))
            return get_function_phrase(offset - 2, count_sentences, argv + 2);
        else {
            if (!get_variable(argv[2], count_sentences))
                return false;
            strcat(sentences_string[count_sentences], variable[count_sentences][0]);
            return get_function_phrase(offset - 3, count_sentences, argv + 3);
        }
    }
    return false;
}

//Function : Process array sentence -> construct sentences_string
//-> return TRUE if this sentence is correct, FALSE otherwise
bool get_array_phrase(int offset, int count_sentences, char **argv) {
    if (strcmp(argv[0], "of"))
        return false;
    //Check the end of array phrase (sentence)
/*
    if (offset < 4)
        return false;
*/
    //If this array OF N DATA|DATUM OF TYPE...
    if (!strcmp(argv[2], "data") || !strcmp(argv[2], "datum")) {
        if (!check_number(argv[1], strlen(argv[1])))
            return false;
        int number = strtoul(argv[1], NULL, 10);
        if (number <= 1 && !strcmp(argv[2], "data"))
            return false;
        if (number != 1 && !strcmp(argv[2], "datum"))
            return false;
        if (strcmp(argv[3], "of") || strcmp(argv[4], "type"))
            return false;
        //If the number of words is smaller than 7 -> level 2 or 3
        if (offset >= 7) {
            if (!strcmp(argv[5], "the") && !strcmp(argv[6], "type") && !strcmp(argv[7], "of")) {
                strcat(sentences_string[count_sentences], "[");
                strcat(sentences_string[count_sentences], argv[1]);
                strcat(sentences_string[count_sentences], "]");
                
                variable[count_sentences][1] = argv[8];
                char *temp = malloc(sizeof(char) * 100);
                strcpy(temp, argv[8]);
                strcat(temp, " ");
                strcat(temp, sentences_string[count_sentences]);
                strcpy(sentences_string[count_sentences], temp);
                return true;
            }
            else
                return false;
        }
        else {
            if (!check_basic_type_description(offset - 4, count_sentences, argv + 4))
                return false;
            strcat(sentences_string[count_sentences], "[");
            strcat(sentences_string[count_sentences], argv[1]);
            strcat(sentences_string[count_sentences], "]");

            char *temp = malloc(sizeof(char) * 100);
            temp = sentences_string[count_sentences];
            //Create new sentences_string
            sentences_string[count_sentences] = malloc(sizeof(char) * 100);
            construct_sentence(offset, count_sentences, argv);
            strcat(sentences_string[count_sentences], temp);
            return true;
        }
    }
    //If this array of ARRAY|ARRAYS
    else if (!strcmp(argv[2], "arrays") || !strcmp(argv[2], "array")){
        if (!check_number(argv[1], strlen(argv[1])))
            return false;
        int number = strtoul(argv[1], NULL, 10);
        if (number <= 1 && !strcmp(argv[2], "arrays"))
            return false;
        if (number != 1 && !strcmp(argv[2], "array"))
            return false;
        strcat(sentences_string[count_sentences], "[");
        strcat(sentences_string[count_sentences], argv[1]);
        strcat(sentences_string[count_sentences], "]");
        return get_array_phrase(offset - 3, count_sentences, argv + 3);
    }
    //If this array of POINTERS|POINTER
    else if (!strcmp(argv[2], "pointers") || !strcmp(argv[2], "pointer")){
        if (!check_number(argv[1], strlen(argv[1])))
            return false;
        int number = strtoul(argv[1], NULL, 10);
        if (number <= 1 && !strcmp(argv[2], "pointers"))
            return false;
        if (number != 1 && !strcmp(argv[2], "pointer"))
            return false;
        
        strcat(sentences_string[count_sentences], "[");
        strcat(sentences_string[count_sentences], argv[1]);
        strcat(sentences_string[count_sentences], "]");
        
        char *temp = malloc(sizeof(char) * 100);
        temp[0] = '*';
        temp[1] = '\0';
        strcat(temp, sentences_string[count_sentences]);
        strcpy(sentences_string[count_sentences], temp);
        
        return get_pointer_phrase(offset - 3, count_sentences, argv + 3);
    }
    return false;
}

//Function : Process POINTER sentence -> construct sentences_string
//-> return TRUE if this sentence is correct, FALSE otherwise
bool get_pointer_phrase(int offset, int count_sentences, char **argv) {
    if (strcmp(argv[0], "to"))
        return false;
    if (!strcmp(argv[1], "void")) {
        char *temp = malloc(sizeof(char) * 100);
        strcpy(temp, argv[1]);
        strcat(temp, " ");
        strcat(temp, sentences_string[count_sentences]);
        strcpy(sentences_string[count_sentences], temp);
        return true;
    }
/*
    if (offset < 4)
        return false;
*/
    //If this pointer to A DATUM|DATA OF TYPE...
    if (!strcmp(argv[1], "a") && !strcmp(argv[2], "datum") && !strcmp(argv[3], "of") && !strcmp(argv[4], "type")) {
        if (offset >= 8) {
            if (!strcmp(argv[5], "the") && !strcmp(argv[6], "type") && !strcmp(argv[7], "of")) {
                variable[count_sentences][1] = argv[8];
                char *temp = malloc(sizeof(char) * 100);
                strcpy(temp, argv[8]);
                strcat(temp, " ");
                strcat(temp, sentences_string[count_sentences]);
                strcpy(sentences_string[count_sentences], temp);
                return true;
            }
            else 
                return false;
        }
        else {
            if (!check_basic_type_description(offset - 4, count_sentences, argv + 4)) {
                return false;
            }
            char *temp = malloc(sizeof(char) * 100);
            temp = sentences_string[count_sentences];
            //Create new sentences_string
            sentences_string[count_sentences] = malloc(sizeof(char) * 100);
            construct_sentence(offset, count_sentences, argv);
            strcat(sentences_string[count_sentences], temp);
            return true;
        }
    }
    else if (!strcmp(argv[1], "data") && !strcmp(argv[2], "of") && !strcmp(argv[3], "type")) {
        if (sentences[count_sentences][1] != 5)
            return false;
        if (offset >= 7) {
            if (!strcmp(argv[4], "the") && !strcmp(argv[5], "type") && !strcmp(argv[6], "of") && offset >= 7) {
                variable[count_sentences][1] = argv[7];
                char *temp = malloc(sizeof(char) * 100);
                strcpy(temp, argv[7]);
                strcat(temp, " ");
                strcat(temp, sentences_string[count_sentences]);
                strcpy(sentences_string[count_sentences], temp);
                return true;
            }
            else
                return false;
        }
        else {
            if (!check_basic_type_description(offset - 3, count_sentences, argv + 3)) {
                return false;
            }
            char *temp = malloc(sizeof(char) * 100);
            temp = sentences_string[count_sentences];
            //Create new sentences_string
            sentences_string[count_sentences] = malloc(sizeof(char) * 100);
            construct_sentence(offset, count_sentences, argv);
            strcat(sentences_string[count_sentences], temp);
            return true;
        }
    }
    //If this pointer to ARRAY|ARRAYS
    else if (!strcmp(argv[1], "arrays")) {
        char *temp = malloc(sizeof(char) * 100);
        temp[0] = '(';
        temp[1] = '\0';
        strcat(temp, sentences_string[count_sentences]);
        strcpy(sentences_string[count_sentences], temp);
        strcat(sentences_string[count_sentences], ")");
        return get_array_phrase(offset - 2, count_sentences, argv + 2);
    }
    else if (!strcmp(argv[1], "an") && !strcmp(argv[2], "array")) {
        char *temp = malloc(sizeof(char) * 100);
        temp[0] = '(';
        temp[1] = '\0';
        strcat(temp, sentences_string[count_sentences]);
        strcpy(sentences_string[count_sentences], temp);
        strcat(sentences_string[count_sentences], ")");
        return get_array_phrase(offset - 3, count_sentences, argv + 3);
    }
    //If this pointer to POINTERS|POINTER
    else if (!strcmp(argv[1], "a") && !strcmp(argv[2], "pointer")) {
        char *temp = malloc(sizeof(char) * 100);
        temp[0] = '*';
        temp[1] = '\0';
        strcat(temp, sentences_string[count_sentences]);
        strcpy(sentences_string[count_sentences], temp);
        return get_pointer_phrase(offset - 3, count_sentences, argv + 3);
    }
    else if (!strcmp(argv[1], "pointers")) {
        char *temp = malloc(sizeof(char) * 100);
        temp[0] = '*';
        temp[1] = '\0';
        strcat(temp, sentences_string[count_sentences]);
        strcpy(sentences_string[count_sentences], temp);
        return get_pointer_phrase(offset - 2, count_sentences, argv + 2);
    }
    //If this pointer to FUNCTION|FUNCTIONS
    else if (!strcmp(argv[1], "a") && !strcmp(argv[2], "function")) {
        char *temp = malloc(sizeof(char) * 100);
        temp[0] = '(';
        temp[1] = '\0';
        strcat(temp, sentences_string[count_sentences]);
        strcpy(sentences_string[count_sentences], temp);
        strcat(sentences_string[count_sentences], ")");
        return get_function_phrase(offset - 3, count_sentences, argv + 3);
    }
    else if (!strcmp(argv[1], "functions")) {
        char *temp = malloc(sizeof(char) * 100);
        temp[0] = '(';
        temp[1] = '\0';
        strcat(temp, sentences_string[count_sentences]);
        strcpy(sentences_string[count_sentences], temp);
        strcat(sentences_string[count_sentences], ")");
        return get_function_phrase(offset - 2, count_sentences, argv + 2);
    }
    
    return false;
}

//Function : Process FUNCTION sentence -> construct sentences_string
//-> return TRUE if this sentence is correct, FALSE otherwise
bool get_function_phrase(int offset, int count_sentences, char **argv) {
    //If this function returning VOID
    if (!strcmp(argv[1], "void") && !strcmp(argv[0], "returning")) {
        char *temp = malloc(sizeof(char) * 100);
        strcpy(temp, argv[1]);
        strcat(temp, " ");
        strcat(temp, sentences_string[count_sentences]);
        strcpy(sentences_string[count_sentences], temp);
        strcat(sentences_string[count_sentences], "()");
        return true;
    }
    if (strcmp(argv[0], "returning") || strcmp(argv[1], "a"))
        return false;
    //If this function return a DATA|DATUM OF TYPE...
    if (offset >= 5 && !strcmp(argv[2], "datum") && !strcmp(argv[3], "of") && !strcmp(argv[4], "type")) {
        if (offset < 5)
            return false;
        if (strcmp(argv[5], "the") && !check_basic_type_description(offset - 5, count_sentences, argv + 5)) {
            return false;
        }
        else if (!strcmp(argv[5], "the") && !strcmp(argv[6], "type") && !strcmp(argv[7], "of")) {
            variable[count_sentences][1] = argv[8];
        }
        strcat(sentences_string[count_sentences], "()");
        if (!variable[count_sentences][1]) {
            char *temp = malloc(sizeof(char) * 100);
            temp = sentences_string[count_sentences];
            sentences_string[count_sentences] = malloc(sizeof(char) * 100);
            construct_sentence(offset, count_sentences, argv);
            strcat(sentences_string[count_sentences], temp);
        }
        else {
            char *temp = malloc(sizeof(char) * 100);
            strcpy(temp, argv[8]);
            strcat(temp, " ");
            strcat(temp, sentences_string[count_sentences]);
            strcpy(sentences_string[count_sentences], temp);
        }
        return true;
    }
    //If this function returning a POINTER
    else {
        if (strcmp(argv[2], "pointer"))
            return false;
        char *temp = malloc(sizeof(char) * 100);
        temp[0] = '*';
        temp[1] = '\0';
        strcat(temp, sentences_string[count_sentences]);
        strcpy(sentences_string[count_sentences], temp);
        strcat(sentences_string[count_sentences], "()");
        return get_pointer_phrase(offset - 3, count_sentences, argv + 3);
    }
    return false;
}

//Check string is satisfies variable definition or not.
//If satisfiable, copy this variable to array variable
bool get_variable(char *string, int count_sentences) {
    if (!strcmp(string, "a") || !strcmp(string, "A") || !strcmp(string, "An"))
        return false;
    if (!check_key_word(string) && check_name_of_argument(string))
        variable[count_sentences][0] = string;
    else 
        return false;
    return true;
}

//Function : Lastly, process all sentences which have reference to others
//Return TRUE if all things are done and FALSE otherwise
bool assigning_variable_fourth_level() {
    /* We travel all sentences
     * If a sentence is referenced by other -> process it
     * This sentence has variable[][0] is NOT NULL and variable[][1] is NULL
     */
    for (int i = 1; i <= NB_OF_SENTENCES; i++) {
        for (int j = 1; j <= NB_OF_SENTENCES; j++) {
            check_variable[j] = false;
        }
        if (!variable[i][1] && variable[i][0]) {
            travel_sentences(i);
        }
    }
    bool check = true;
    /* After processing all sentences, we need to check again
     * Check all sentences are correct or not
     * Check if there are two sentences which have a same variable with different BASIC TYPES -> WRONG
     */
    for (int j = 1; j <= NB_OF_SENTENCES; j++) {
        if (variable[j][1])
            check = false;
    }
    
    for (int j = 1; j <= NB_OF_SENTENCES; j++) {
        if (!variable[j][0])
            continue;
        for (int i = 1; i <= NB_OF_SENTENCES; i++) {
            if (variable[i][0] && !strcmp(variable[i][0], variable[j][0]) && array_variable[i] && i != j) {
                check = false;
            }
        }
    }
    return check;
}

//Function : Travel to each sentence, if this sentence reference to other one -> process it
void travel_sentences(int index) {
    if (!variable[index][0])
        return EXIT_SUCCESS;
    check_variable[index] = true;
    for (int i = 1; i <= NB_OF_SENTENCES; i++) {
        if (!check_variable[i] && variable[i][1]) {
            if (!strcmp(variable[i][1], variable[index][0]) && !variable[index][1]) {
                string_combining(sentences_string[i], i, sentences_string[index], index);
                array_variable[index] = true;
                travel_sentences(i);
            }
        }
    }
}

//Function : Concatenate the SOURCE sentence to DESTINATION sentence IF:
//DESTINATION sentence reference to SOURCE sentence
void string_combining(char *des, int des_index, char *sour, int sour_index) {
    char *string = malloc(sizeof(char) * 100);
    int i = 0, j = 0;
    int count = 0;
    for (i = 0; i < strlen(sour); i++) {
        if (variable[sour_index][0])
            if (sour[i] == variable[sour_index][0][0])
                break;
        string[i] = sour[i];
    }
    if (sour[i - 1] == ' ')
        count++;
    int next_index = i + strlen(variable[des_index][1]);
    for (j = 0; j < strlen(des); j++) {
        if (des[j] == ' ')
            break;
    }
    j++;
    bool check = false;
    if (sour[next_index] != '\0' && sour[next_index] != ')' && des[j] != '[') {
        string[i++] = '(';
        check = true;
    }
    while (des[j] != '\0') {
        string[i++] = des[j++];
    }
    //printf("%c %c ", string[i - 1], sour[next_index - 1]);
    if (string[i - 1] == ')')
        count++;
    if (sour[next_index] == '[')
        count++;
    
    if (check) {
        string[i++] = ')';
    }
    while (sour[next_index] != '\0') {
        string[i++] = sour[next_index++];
    }
    string[i] = '\0';
    if (sentences_string[des_index])
        strcpy(sentences_string[des_index], string);
    //If the sour sentence is a function and the destination is an array -> WRONG INPUT
    //This means count == 3
    if (count != 3)
        variable[des_index][1] = NULL;
}
