#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils.h"

/**
 * @brief Conta il numero totale di files in una directory
 *
 * @param directory puntatore all'oggetto directory di cui contare la totalità dei files
 * @return numero di file di tipo regolare contenuti nella directory
 */
int get_directory_total_files(DIR *directory)
{
    // controllo che la directory sia valida
    if (!directory)
    {
        printf("La directory selezionata non è valida\n");
        return -1;
    }
    int file_count = 0;
    struct dirent *entry;
    // itero attraverso il contenuto della directory
    // ad ogni iterazione readdir() restituirà un file diverso all'interno della directory
    while ((entry = readdir(directory)) != NULL)
    {
        // conto solamente i file di tipo regolare
        if (entry->d_type != DT_REG)
            continue;
        file_count++;
    }
    // vado a riportare il puntatore del flusso nella directory all'inizio dato che le voci di questa directory potrebbero
    // essre re iterate successivamente tramite readdir

    rewinddir(directory);
    return file_count;
}

/**
 * @brief Carica il quiz presente all'interno di un file e crea la struttura Quiz corrispondente
 *
 * Questa funzione si occupa di andare a creare la struttura Quiz relativa al quiz all'interno del file file_path.
 * Questa funzione si occupa della decodifica del file secondo un formato prefissato e dell'allocazione e riempimento di tutti i campi della struttura Quiz
 *
 * @param file_path puntatore al path del file che contiene le informazioni del quiz
 * @return struttura di tipo Quiz relativa al file file_path
 */
Quiz *load_quiz_from_file(const char *file_path)
{
    FILE *file = fopen(file_path, "r");
    if (!file)
    {
        printf("Errore nell'apertura del file\n");
        exit(EXIT_FAILURE);
    }
    // alloco il quiz
    Quiz *quiz = (Quiz *)malloc(sizeof(Quiz));
    if (!quiz)
    {
        printf("Errore di allocazione memoria per il quiz\n");
        exit(EXIT_FAILURE);
    }

    // puntatore alla linea utilizzato da getline
    char *lineptr = NULL;
    // dimensione corrente del buffer lineptr
    size_t len = 0;
    // numero di caratteri letti, incluso \n, ma escluso \0
    ssize_t read;

    int question_idx = 0;
    QuizQuestion *current_question = NULL;
    int answer_idx = 0;
    char *token, *answers_line, *answer_line_copy;

    int question_count = 0;

    quiz->questions = NULL;
    quiz->ranking_head = NULL;
    quiz->ranking_tail = NULL;
    quiz->total_clients = 0;

    // leggo la prima linea, relativa al nome del quiz
    if ((read = getline(&lineptr, &len, file)) != -1)
    {
        // rimuovo il carattere di nuova linea e lo sostituisco con il terminatore
        lineptr[strcspn(lineptr, "\n")] = '\0';
        // duplico la stringa, incluso il terminatore in quiz->name
        quiz->name = strdup(lineptr);
    }

    // conto il numero totale di domande contando quelle che sono le righe che contengono la parola domanda
    while ((read = getline(&lineptr, &len, file)) != -1)
    {
        // strstr cerca la prima occorrenza di una sottostringa in una stringa
        if (strstr(lineptr, "Domanda"))
            question_count++;
    }

    quiz->total_questions = question_count;

    // alloco gli array per domande e risposte
    quiz->questions = (QuizQuestion **)malloc(question_count * sizeof(QuizQuestion *));

    if (!quiz->questions)
    {
        printf("Errore nell'allocazione della memoria per le domande del quiz\n");
        exit(EXIT_FAILURE);
    }

    // torno all'inizio del file per leggere le domande e risposte
    rewind(file);
    // salto la prima linea che contiene il nome del quiz
    getline(&lineptr, &len, file);

    // ciclo in maniera infinita attraverso il file in modo da poter leggere due righe alla volta
    while (true)
    {
        // leggo la linea vuota prima della domanda
        read = getline(&lineptr, &len, file);

        // leggo la riga che dovrebbe contenere la domanda
        read = getline(&lineptr, &len, file);

        if (read == -1)
            // il file è finito
            break;

        // inserisco al posto del carattere \n il terminatore di stringa \0
        lineptr[strcspn(lineptr, "\n")] = '\0';

        // se la riga non è quella corretta
        if (strncmp(lineptr, "Domanda: ", 9) != 0)
        {
            printf("Il file del quiz %s non è formattato correttamente, consulta la documentazione\n", quiz->name);
            exit(EXIT_FAILURE);
        }
        // alloco l'oggetto relativo alla domanda corrente
        current_question = (QuizQuestion *)malloc(sizeof(QuizQuestion));
        if (current_question == NULL)
        {
            printf("Errore nell'allocazione della domanda del quiz\n");
            exit(EXIT_FAILURE);
        }
        current_question->question = strdup(lineptr + 9);
        current_question->total_answers = 0;

        // leggo la linea successiva che dovrebbe contenere le risposte
        read = getline(&lineptr, &len, file);

        lineptr[strcspn(lineptr, "\n")] = '\0';

        if (read == -1 || strncmp(lineptr, "Risposte: ", 10) != 0)
        {
            // in questo caso si ha una domanda senza le risposte
            printf("Il file del quiz %s non è formattato correttamente, consulta la documentazione\n", quiz->name);
            exit(EXIT_FAILURE);
        }
        // salto la stringa "Risposte: "
        answers_line = lineptr + 10;

        // creo una copia della linea con le risposte perché strtok modifica la riga di partenza
        answer_line_copy = strdup(answers_line);

        // conto quante sono le risposte possibili
        token = strtok(answer_line_copy, ",");
        while (token)
        {
            current_question->total_answers += 1;
            token = strtok(NULL, ",");
        }
        free(answer_line_copy);

        // alloco il vettore che conterrà tutte le risposte
        current_question->answers = malloc(current_question->total_answers * sizeof(char *));
        if (!current_question->answers)
        {
            printf("Errore nell'allocazione della memoria per le risposte ad una domanda\n");
            exit(EXIT_FAILURE);
        }

        // inserisco le risposte effettivamente nel array
        token = strtok(answers_line, ",");
        answer_idx = 0;
        while (token)
        {
            while (*token == ' ')
                token += 1;
            current_question->answers[answer_idx] = strdup(token);
            answer_idx += 1;
            token = strtok(NULL, ",");
        }
        quiz->questions[question_idx++] = current_question;
    }

    free(lineptr);
    fclose(file);

    return quiz;
}

/**
 * @brief Carica i quiz presenti all'interno di una directory
 *
 * Questa funzione si occupa di andare a caricare tutti i quiz presenti all'interno di una directory specifica utilizzando
 * la funzione load_quiz_from_file su tutti i file presenti all'interno della directory.
 * I quiz verranno inseriti all'interno dell'oggetto quizzesInfo per fare in modo che tutta l'applicazione ne possa usufruire.
 *
 * @param directory_path path della directory da cui prelevare i quizzes
 * @param quizzesInfo puntatore alla struttura all'interno del quale inserire i quiz caricati
 * @return numero di quiz caricati
 */
int load_quizzes_from_directory(const char *directory_path, QuizzesInfo *quizzesInfo)
{
    // creo la struttura relativa alla directory e apro il path
    struct dirent *entry;
    DIR *directory = opendir(directory_path);
    char file_path[PATH_MAX];
    int current_quiz = 0;

    if (directory == NULL)
    {
        printf("Errore nell'apertura della directory specificata\n");
        exit(EXIT_FAILURE);
    }

    // conto il numero totale dei quiz presenti nella directory
    quizzesInfo->total_quizzes = get_directory_total_files(directory);

    // alloco un array di puntatori a quiz che verranno popolati leggendo i files della directory
    quizzesInfo->quizzes = (Quiz **)malloc((quizzesInfo->total_quizzes) * sizeof(Quiz *));

    if (!quizzesInfo->quizzes)
    {
        printf("Errore di allocazione memoria per l'array dei quiz\n");
        exit(EXIT_FAILURE);
    }

    // itero attraverso i file della directory
    while ((entry = readdir(directory)) != NULL)
    {
        // considero solamente i file di tipo regolare
        if (entry->d_type != DT_REG)
            continue;

        // inserisco all'interno di file_path il path completo del file da da cui prelevare il quiz
        snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);

        // prelevo il quiz dal file file_path e lo inserisco nell'array

        quizzesInfo->quizzes[current_quiz] = load_quiz_from_file(file_path);

        current_quiz++;
    }

    // chiudo la directory
    closedir(directory);
    return quizzesInfo->total_quizzes;
}

/**
 * @brief Dealloca tutte le informazioni relative ai quiz
 *
 * @param quizzesInfo puntatore alla struttura all'interno del quale sono contenuti tutti i quizzes
 */
void deallocate_quizzes(QuizzesInfo *quizzesInfo)
{
    for (uint16_t i = 0; i < quizzesInfo->total_quizzes; i++)
    {
        Quiz *quiz = quizzesInfo->quizzes[i];
        if (!quiz)
            continue;
        free(quiz->name);

        // dealloco la lista doppiamente concatenata relativa alla classifica del quiz
        deallocate_rankings(quiz);

        for (uint16_t j = 0; j < quiz->total_questions; j++)
        {
            QuizQuestion *question = quiz->questions[j];
            if (!question)
                continue;

            free(question->question);

            for (int k = 0; k < question->total_answers; k++)
                free(question->answers[k]);

            free(question->answers);
            free(question);
        }
        free(quiz->questions);

        free(quiz);
    }
    free(quizzesInfo->quizzes);
}