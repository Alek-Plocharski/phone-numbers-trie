/** @file
 * Interfejs tekstowy obsługujący wiele baz przekierowań numerów telefonów.
 *
 * @author Aleksander Płocharski <ap394689@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 09.04.2018
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "phone_forward.h"

#define ERROR 3 /**<informuję o błędzie wystąpieniu błędu składniowego we wczytywaniu komentarza */
#define SUCCESS 4 /**<informuję o sukcesie wczytania komentarza */
#define STARTING_SIZE 10 /**< początkowy rozmiar tablicy na wczytywanie liczb i identyfikatorów */
#define MULTIPLIER 3 /**<licznik mnożnika rozmiaru tablicy do wczytywania liczb i identyfikatorów */
#define DIVISOR 2 /**<mianownik mnożnika rozmiaru tablicy do wczytywania liczb i identyfikatorów */
#define NOTHING_LOADED 1 /**< informuję o nie wczytaniu, żadnego znaku przy wczytywaniu białych znaków i komentarzy */
#define SUCCESSFULLY_LOADED 2 /**< informuję o poprawnym wczytaniu białych znaków i komentarzy (przynajmniej jeden znak wczytany) */
#define NUMBER_OF_DIGITS 12 /**<liczba znaków uznawanych za cyfry */

/** @brief Struktura przechowująca listę baz przekierowań.
 * Struktura przechowuję bazy przekierowań w formie listy.
 * Każdy element zawiera identyfikator bazy, wskaźnik na drzewo przekierowań i wskaźnik na następny element.
 */
struct ForwardTreeList {

    struct ForwardTreeList *next; /**< wskaźnik na następny element  */
    char *id; /**< wskaźnik na identyfikator */
    struct PhoneForward *pf; /**< wskaźnik na drzewo przekierowań */
};

/** @brief Tworzy nowy element listy baz przekierowań o podanym identyfikatorze.
 * @param[in] id - wskaźnik na identyfikator.
 * @return Wskaźnik na nowo powstały element lub NULL w przypadku błędu alokacji.
 */
static struct ForwardTreeList *newFwdTreeListElement(const char *id) {

    struct ForwardTreeList *pfListElement = malloc(sizeof(struct ForwardTreeList));

    if (pfListElement != NULL) {

        pfListElement->next = NULL;
        pfListElement->id = malloc(sizeof(char) * (strlen(id) + 1));

        if (pfListElement->id == NULL) {
            free(pfListElement);
            return NULL;
        }

        strcpy(pfListElement->id, id);
        pfListElement->pf = phfwdNew();
    }

    return pfListElement;
}

/** @brief Zwalnia element listy baz przekierowań.
 * @param[in,out] element - wskaźnik na zwalniany element.
 */
static void delForwardListElement(struct ForwardTreeList *element) {

    free(element->id);
    phfwdDelete(element->pf);
    free(element);
}

/** @brief Zwalnia listę baz przekierowań.
 * @param[in,out] pfList - wskaźnik na zwalnianą listę.
 */
static void delFwdTreeList(struct ForwardTreeList *pfList) {

    while (pfList != NULL) {

        struct ForwardTreeList *tmp = pfList;
        free(tmp->id);
        phfwdDelete(tmp->pf);
        pfList = pfList->next;
        free(tmp);
    }
}

/** @brief Dodaję bazę do listy baz przekierowań.
 * Dodaje bazę o podanym identyfikatorze do listy baz przekierowań. Jeśli baza o takim identyfikatorze już istnieje
 * ustawia ją jako aktualną bazę.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in] id - wskaźnik na identyfikator;
 * @param[in,out] currentFwdTree - adres wskaźnika na aktualną bazę.
 * @return Wartość @p true jeśli dodanie powiodło się,
 *         wartość @p false, gdy wystąpił problem z alokacją pamięci.
 */
static bool addToForwardTreeList(struct ForwardTreeList **pfList, const char *id, struct ForwardTreeList **currentFwdTree) {

    if ((*pfList) == NULL) {

        (*pfList) = newFwdTreeListElement(id);
        (*currentFwdTree) = (*pfList);
        return ((*pfList) != NULL);
    }

    if (strcmp((*pfList)->id, id) == 0) {
        (*currentFwdTree) = (*pfList);
        return true;
    }

    struct ForwardTreeList* tmp = (*pfList);

    while (tmp->next != NULL && strcmp(tmp->next->id, id) != 0)
        tmp = tmp->next;

    if (tmp->next != NULL) {

        (*currentFwdTree) = tmp->next;
        return true;
    }

    tmp->next = newFwdTreeListElement(id);
    (*currentFwdTree) = tmp->next;
    return (tmp->next != NULL);
}

/** @brief Usuwa bazę z listy baz przekierowań.
 * Usuwa bazę o podanym identyfikatorze z listy baz przekierowań. Jeśli usuwana baza jest również bazą aktualną
 * ustawia wskaźnik na aktualną bazę na NULL.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in] id - wskaźnik na identyfikator;
 * @param[in,out] currentFwdTree - adres wskaźnika na aktualną bazę.
 * @return Wartość @p true jeśli usuwanie powiodło się,
 *         wartość @p false jeśli baza o podanym identyfikatorze nie jest na liście.
 */
static bool delFromForwardTreeList(struct ForwardTreeList **pfList, const char *id, struct ForwardTreeList **currentFwdTree) {

    if ((*pfList) == NULL)
        return false;

    if (strcmp((*pfList)->id, id) == 0) {

        struct ForwardTreeList *tmp = (*pfList);
        (*pfList) = (*pfList)->next;

        if ((*currentFwdTree) == tmp)
            (*currentFwdTree) = NULL;

        delForwardListElement(tmp);
        return true;
    }

    if ((*pfList)->next == NULL)
        return false;

    struct ForwardTreeList* tmp = (*pfList)->next;
    struct ForwardTreeList* prev = (*pfList);

    while (tmp != NULL && strcmp(tmp->id, id) != 0) {
        tmp = tmp->next;
        prev = prev->next;
    }

    if (tmp == NULL)
        return false;

    prev->next = tmp->next;

    if ((*currentFwdTree) == tmp)
        (*currentFwdTree) = NULL;

    delForwardListElement(tmp);

    return true;
}

/** @brief Wykonuję komendę dodania bazy.
 * Dodaję bazę przekierowań o podanym identyfikatorze i ustawia ją jako aktualną. Jeżeli taka baza już istnieje
 * tylko ustawia ją jako aktualną.  W przypadku błędu wykonania komendy wypisuję
 * stosowny błąd i kończy działanie programu.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] id - wskaźnik na identyfikator;
 * @param[in] byteNumber - numer pierwszego znaku wywołanego operatora;
 * @param[in,out] currentFwdTree - adres wskaźnika na aktualną bazę.
 */
static void addForwardBase(struct ForwardTreeList **pfList, const char *id, int byteNumber, struct ForwardTreeList **currentFwdTree) {

    bool result = addToForwardTreeList(pfList, id, currentFwdTree);

    if (result == false) {

        fprintf(stderr, "ERROR NEW %d\n", byteNumber);
        free((void *) id);
        delFwdTreeList((*pfList));
        exit(1);
    }
}

/** @brief Wykonuję komendę usunięcia bazy.
 * Usuwa bazę przekierowań o podanym identyfikatorze. W przypadku błędu wykonania komendy wypisuję
 * stosowny błąd i kończy działanie programu.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] id - wskaźnik na identyfikator;
 * @param[in] byteNumber - numer pierwszego znaku wywołanego operatora;
 * @param[in,out] currentFwdTree - adres wskaźnika na aktualną bazę.
 */
static void delForwardBase(struct ForwardTreeList **pfList, const char *id, int byteNumber, struct ForwardTreeList **currentFwdTree) {

    bool result = delFromForwardTreeList(pfList, id, currentFwdTree);

    if (result == false) {

        fprintf(stderr, "ERROR DEL %d\n", byteNumber);
        free((void *) id);
        delFwdTreeList((*pfList));
        exit(1);
    }
}

/** @brief Wykonuję komendę dodania przekierowania.
 * Dodaje dane dane przekierowanie do drzewa przekierowań aktualnej bazy.
 * W przypadku błędu wykonania komendy wypisuję stosowny błąd i kończy działanie programu.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] from - wskaźnik na numer, z którego jest przekierowanie;
 * @param[in,out] to - wskaźnik na numer na który jest przekierowanie;
 * @param[in] byteNumber - numer pierwszego znaku wywołanego operatora;
 * @param[in,out] currentFwdTree - wskaźnik na aktualną bazę.
 */
static void addForward(struct ForwardTreeList **pfList, const char *from, const char *to, int byteNumber, struct ForwardTreeList *currentFwdTree) {

    if (currentFwdTree == NULL) {

        fprintf(stderr, "ERROR > %d\n", byteNumber);
        free((void *) from);
        free((void *) to);
        delFwdTreeList((*pfList));
        exit(1);
    }

    bool result = phfwdAdd(currentFwdTree->pf, from, to);

    if (result == false) {

        fprintf(stderr, "ERROR > %d\n", byteNumber);
        free((void *) from);
        free((void *) to);
        delFwdTreeList((*pfList));
        exit(1);
    }
}

/** @brief Wykonuje komendę usunięcia przekierowań.
 * Usuwa z aktualnej bazy przekierowania o podanym prefiksie.
 * W przypadku błędu wykonania komendy wypisuję stosowny błąd i kończy działanie programu.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] num - wskaźnik na numer będącym prefiksem z jakim przekierowania mają zostać usunięte;
 * @param[in] byteNumber - numer pierwszego znaku wywołanego operatora;
 * @param[in,out] currentFwdTree - wskaźnik na aktualną bazę.
 */
static void removeForwards(struct ForwardTreeList **pfList, const char *num, int byteNumber, struct ForwardTreeList *currentFwdTree) {

    if (currentFwdTree == NULL) {

        fprintf(stderr, "ERROR DEL %d\n", byteNumber);
        free((void *) num);
        delFwdTreeList((*pfList));
        exit(1);
    }

    phfwdRemove(currentFwdTree->pf, num);
}

/** @brief Wykonuje komendę wypisania przekierowania z danego numeru.
 * Wypisuje przekierowanie podanego numeru w aktualnej bazie.
 * W przypadku błędu wykonania komendy wypisuję stosowny błąd i kończy działanie programu.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] num - wskaźnik na numer;
 * @param[in] byteNumber - numer pierwszego znaku wywołanego operatora;
 * @param[in,out] currentFwdTree - wskaźnik na aktualną bazę.
 */
static void getForward(struct ForwardTreeList **pfList, const char *num, int byteNumber, struct ForwardTreeList *currentFwdTree) {

    if (currentFwdTree == NULL) {

        fprintf(stderr, "ERROR ? %d\n", byteNumber);
        free((void *) num);
        delFwdTreeList((*pfList));
        exit(1);
    }

    const struct PhoneNumbers* pnum = phfwdGet(currentFwdTree->pf, num);

    printf("%s\n", phnumGet(pnum, 0));
    phnumDelete(pnum);
}

/** @brief Wykonuje komendę wypisania przekierowań na dany numer.
 * Wypisuję numery, które przekierowują się na dany numer w aktualnej bazie.
 * W przypadku błędu wykonania komendy wypisuję stosowny błąd i kończy działanie programu.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] num - wskaźnik na numer;
 * @param[in] byteNumber - numer pierwszego znaku wywołanego operatora;
 * @param[in,out] currentFwdTree - wskaźnik na aktualną bazę.
 */
static void getReverse(struct ForwardTreeList **pfList, const char *num, int byteNumber, struct ForwardTreeList *currentFwdTree) {

    if (currentFwdTree == NULL) {

        fprintf(stderr, "ERROR ? %d\n", byteNumber);
        free((void *) num);
        delFwdTreeList((*pfList));
        exit(1);
    }

    const struct PhoneNumbers* pnum = phfwdReverse(currentFwdTree->pf, num);

    size_t index = 0;
    const char *number = phnumGet(pnum, index);

    while (number != NULL) {

        printf("%s\n", phnumGet(pnum, index));
        index++;
        number = phnumGet(pnum, index);
    }

    phnumDelete(pnum);
}

/** @brief Wykonuję komendę zliczania nietrywialnych numerów.
 * Wypisuję rezultat wywołania funkcji @ref phfwdNonTrivialCount na aktualnej bazie przekierowań.
 * W razie błędu wykonania wypisuję stosowny komunikat i kończy działanie programu.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] num - wskaźnik na numer;
 * @param[in] byteNumber - numer pierwszego znaku wywołanego operatora;
 * @param[in,out] currentFwdTree - wskaźnik na aktualną bazę.
 */
static void getNonTrivialCount(struct ForwardTreeList **pfList, const char *num, int byteNumber, struct ForwardTreeList *currentFwdTree) {

    if (currentFwdTree == NULL) {

        fprintf(stderr, "ERROR @ %d\n", byteNumber);
        free((void *) num);
        delFwdTreeList((*pfList));
        exit(1);
    }

    size_t len = strlen(num);

    len = (len > NUMBER_OF_DIGITS ? len - NUMBER_OF_DIGITS : 0);

    size_t solution = phfwdNonTrivialCount(currentFwdTree->pf, num, len);

    printf("%zu\n", solution);
}

/** @brief Sprawdza czy znak jest białym znakiem
 * Sprawdza czy znak jest białym znakiem według wymogów zadania.
 * @param ch - sprawdzany znak.
 * @return Wartość @p true jeśli znak jest biały,
 *         wartość @p false jeśli znak nie jest biały.
 */
static bool isWhiteSgn(char ch) {

    return (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t');
}

/** @brief Sprawdza czy znak jest cyfrą.
 * Sprawdza czy znak jest cyfrą według wymogów zadania.
 * @param[in] ch - sprawdzany znak.
 * @return Wartość @p true jeśli znak jest cyfrą,
 *         wartość @p false jeśli znak nie jest cyfrą.
 */
static bool isDigit(char ch) {

    return (ch >= '0' && ch <= ';');
}

/** @brief Kończy program odpowiednim błędem.
 * Funkcja decyduję jaki błąd powinien być wypisany, po czym kończy działanie programu.
 * @param[in,out] pfList - wskaźnik na listę baz przekierowań;
 * @param[in] ch - znak decydujący o rodzaju błędu;
 * @param[in] byteNumber - wskaźnik na licznik wczytanych znaków;
 */
static void errorInputOrEof(struct ForwardTreeList *pfList, char ch, int byteNumber) {

    delFwdTreeList(pfList);
    if (ch == EOF)
        fprintf(stderr, "ERROR EOF\n");

    else
        fprintf(stderr, "ERROR %d\n", byteNumber);

    exit(1);
}

/** @brief Wczytuję komentarz.
 * Wyczytuję pojedynczy komentarz. Jeśli w czasie wczytywania wystąpi błąd (komentarz nie jest zakończony)
 * zwraca odpowiednią wartość.
 * @param[in,out] byteNumber - wskaźnik na licznik wczytanych znaków;
 * @return SUCCESS jeśli komentarz został pomyślnie wczytany,
 *         ERROR jeśli wystąpił błąd we wczytywaniu komentarza.
 */
static int loadComment(int *byteNumber) {

    bool endOfComment = false;
    char ch = getchar();
    (*byteNumber)++;

    if (ch != '$') {

        ungetc(ch, stdin);
        (*byteNumber)--;
        return ERROR;
    }
    ch = getchar();
    (*byteNumber)++;

    while (!endOfComment) {

        while (ch != '$' && ch != EOF) {
            ch = getchar();
            (*byteNumber)++;
        }

        ch = getchar();
        (*byteNumber)++;

        if (ch == EOF)
            return ERROR;

        if (ch == '$')
            endOfComment = true;
    }

    return SUCCESS;
}

/** @brief Wczytuję białe znaki i komentarze.
 * Wczytuję wszystkie białe znaki i komentarze na wejściu aż do napotkania znaku nie zaliczającego się
 * do podanych kategorii. W przypadku błędu we wczytywaniu komentarza zwracana jest odpowiednia wartość.
 * @param[in,out] byteNumber - wskaźnik na licznik wczytanych znaków;
 * @return NOTHING_LOADED jeśli żaden znak nie został wczytany,
 *         SUCCESSFULLY_LOADED jeśli coś zostało wczytane i wczytywanie powiodło się,
 *         ERROR jeśli w czasie wczytywania komentarza wystąpił błąd.
 */
static int loadWhiteSpacesAndComments(int *byteNumber) {

    int result;
    char ch = getchar();
    bool end = false;

    if (ch != '$' && !isWhiteSgn(ch)) {

        ungetc(ch, stdin);
        return NOTHING_LOADED;
    }

    (*byteNumber)++;

    while (!end) {

        while (isWhiteSgn(ch)) {
            ch = getchar();
            (*byteNumber)++;
        }

        if (ch == '$') {

            result = loadComment(byteNumber);
            if (result != SUCCESS)
                return result;

            ch = getchar();
            (*byteNumber)++;
        }

        if (!isWhiteSgn(ch) && ch != '$') {
            end = true;
            ungetc(ch, stdin);
            (*byteNumber)--;
        }
    }

    return SUCCESSFULLY_LOADED;
}

/** @brief Wczytuję liczbę.
 * Wczytuję liczbę do tablicy znaków.
 * Przy wywołaniu funkcji pierwszy wczytany znak jest cyfrą.
 * @param[in,out] byteNumber - wskaźnik na licznik wczytanych znaków;
 * @return Wskaźnik na wczytaną liczbę.
 */
static const char *loadNumber(int *byteNumber) {

    char *num = malloc(sizeof(char) * STARTING_SIZE);

    if (num == NULL)
        return num;

    char *extended;
    size_t size = STARTING_SIZE;
    size_t i = 0;
    char ch = getchar();
    (*byteNumber)++;

    while (isDigit(ch)) {

        if ((i + 1) == size) {

            extended = realloc(num, (MULTIPLIER * size) / DIVISOR);
            size = (MULTIPLIER * size) / DIVISOR;

            if (extended == NULL) {
                ungetc(ch, stdin);
                free(num);
                return extended;
            }

            else
                num = extended;
        }

        num[i] = ch;
        i++;
        ch = getchar();
        (*byteNumber)++;
    }

    num[i] = '\0';

    ungetc(ch, stdin);
    (*byteNumber)--;

    return num;
}

/** @brief Wczytuję identyfikator.
 * Wczytuję identyfikator do tablicy znaków.
 * Przy wywołaniu funkcji pierwszy wczytany znak jest literą.
 * @param[in,out] byteNumber - wskaźnik na licznik wczytanych znaków;
 * @return Wskaźnik na wczytany identyfikator.
 */
static const char *loadId(int *byteNumber) {

    char *id = malloc(sizeof(char) * STARTING_SIZE);

    if (id == NULL)
        return id;

    char *extended;
    size_t size = STARTING_SIZE;
    size_t i = 0;
    char ch = getchar();
    (*byteNumber)++;

    while (isalnum(ch)) {

        if ((i + 1) == size) {

            extended = realloc(id, MULTIPLIER * size / DIVISOR);
            size = (MULTIPLIER * size) / DIVISOR;

            if (extended == NULL) {
                ungetc(ch, stdin);
                free(id);
                return extended;
            }

            else
                id = extended;
        }

        id[i] = ch;
        i++;
        ch = getchar();
        (*byteNumber)++;
    }

    id[i] = '\0';

    ungetc(ch, stdin);
    (*byteNumber)--;

    return id;
}

/** @brief Wczytuję dalszą część komendy dodawania bazy i wykonuję ją.
 * Funkcja wczytuję dalszą część komendy dodawania bazy przekierowań i wykonuję ją.
 * W razie jakichkolwiek błędów składniowych, bądź wykonania komendy kończy działanie programu i wypisuje stosowny błąd.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] byteNumber - wskaźnik na licznik wczytanych znaków;
 * @param[in] startingByte - numer pierwszego znaku wczytanego operatora;
 * @param[in,out] currentFwdTree - adres wskaźnika na aktualną bazę przekierowań.
 */
static void tryNewCommand(struct ForwardTreeList **pfList, int *byteNumber, int startingByte, struct ForwardTreeList **currentFwdTree) {

    char ch = getchar();
    (*byteNumber)++;

    if (ch != 'E')
        errorInputOrEof((*pfList), ch, (*byteNumber));

    ch = getchar();
    (*byteNumber)++;

    if (ch != 'W')
        errorInputOrEof((*pfList), ch, (*byteNumber));

    int result = loadWhiteSpacesAndComments(byteNumber);
    if (result == NOTHING_LOADED || result == ERROR) {

        ch = getchar();
        (*byteNumber)++;
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }

    ch = getchar();
    (*byteNumber)++;

    if(!isalpha(ch))
        errorInputOrEof((*pfList), ch, (*byteNumber));

    ungetc(ch, stdin);
    (*byteNumber)--;

    const char* id = loadId(byteNumber);

    if (id == NULL) {
        ch = getchar();
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }

    if (strcmp(id, "DEL") == 0 || strcmp(id, "NEW") == 0) {

        ch = getchar();
        (*byteNumber)++;

        free((void *) id);
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }

    addForwardBase(pfList, id, startingByte, currentFwdTree);

    free((void *) id);
}

/** @brief Wczytuję dalszą część komendy usuwania bazy przekierowań i wykonuję ją.
 * Funkcja wczytuję dalszą część komendy usuwania bazy przekierowań, po czym wykonuję ją.
 * W razie jakichkolwiek błędów składniowych, bądź wykonania komendy kończy działanie programu i wypisuje stosowny błąd.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] byteNumber - wskaźnik na licznik wczytanych znaków;
 * @param[in] startingByte - numer pierwszego znaku wczytanego operatora;
 * @param[in,out] currentFwdTree - adres wskaźnika na aktualną bazę przekierowań.
 */
static void tryDelBaseCommand(struct ForwardTreeList **pfList, int *byteNumber, int startingByte, struct ForwardTreeList **currentFwdTree) {

    char ch;
    const char *id = loadId(byteNumber);

    if (id == NULL) {
        ch = getchar();
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }

    if (strcmp(id, "DEL") == 0 || strcmp(id, "NEW") == 0) {

        ch = getchar();
        (*byteNumber)++;

        free((void *) id);
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }

    delForwardBase(pfList, id, startingByte, currentFwdTree);
    free((void *) id);
}

/** @brief Wczytuję dalszą część komendy usuwania przekierowań i wykonuję ją.
 * Funkcja wczytuję dalszą część komendy usuwania przekierowań, po czym wykonuję ją.
 * W razie jakichkolwiek błędów składniowych, bądź wykonania komendy kończy działanie programu i wypisuje stosowny błąd.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] byteNumber - wskaźnik na licznik wczytanych znaków;
 * @param[in] startingByte - numer pierwszego znaku wczytanego operatora;
 * @param[in,out] currentFwdTree - adres wskaźnika na aktualną bazę przekierowań.
 */
static void tryDelForwardCommand(struct ForwardTreeList **pfList, int *byteNumber, int startingByte, struct ForwardTreeList **currentFwdTree) {

    char ch;
    const char* num = loadNumber(byteNumber);

    if (num == NULL) {

        ch = getchar();
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }

    removeForwards(pfList, num, startingByte, (*currentFwdTree));
    free((void *) num);
}

/** @brief Wczytuję dalszą część komendy usunięcia przekierowań lub bazy i wykonuję ją.
 * Funkcja wczytuję dalszą część komendy i w trakcie działania determinuję, czy ma do czynienia z usuwaniem bazy,
 * czy usuwaniem przekierowań. Wczytuję komendę do końca i wykonuję ją.
 * W razie jakichkolwiek błędów składniowych, bądź wykonania komendy kończy działanie programu i wypisuje stosowny błąd.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] byteNumber - wskaźnik na licznik wczytanych znaków;
 * @param[in] startingByte - numer pierwszego znaku wczytanego operatora;
 * @param[in,out] currentFwdTree - adres wskaźnika na aktualną bazę przekierowań.
 */
static void tryDelCommand(struct ForwardTreeList **pfList, int *byteNumber, int startingByte, struct ForwardTreeList **currentFwdTree) {

    char ch = getchar();
    (*byteNumber)++;

    if (ch != 'E')
        errorInputOrEof((*pfList), ch, (*byteNumber));

    ch = getchar();
    (*byteNumber)++;

    if (ch != 'L')
        errorInputOrEof((*pfList), ch, (*byteNumber));

    int result = loadWhiteSpacesAndComments(byteNumber);
    if (result == NOTHING_LOADED || result == ERROR) {

        ch = getchar();
        (*byteNumber)++;
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }

    ch = getchar();
    (*byteNumber)++;

    if (isDigit(ch)) {

        ungetc(ch, stdin);
        (*byteNumber)--;

        tryDelForwardCommand(pfList, byteNumber, startingByte, currentFwdTree);
    }

    else if (isalpha(ch)) {

        ungetc(ch, stdin);
        (*byteNumber)--;

        tryDelBaseCommand(pfList, byteNumber, startingByte, currentFwdTree);
    }

    else
        errorInputOrEof((*pfList), ch, (*byteNumber));
}

/** @brief Wczytuję dalszą część komendy wypisania przekierowań na numer i ją wykonuje.
 * Funkcja wczytuje komendę reverse po po czym wykonuję ją.
 * W razie jakichkolwiek błędów składniowych, bądź wykonania komendy kończy działanie programu i wypisuje stosowny błąd.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] byteNumber - wskaźnik na licznik wczytanych znaków;
 * @param[in] startingByte - numer pierwszego znaku wczytanego operatora;
 * @param[in,out] currentFwdTree - adres wskaźnika na aktualną bazę przekierowań.
 */
static void tryReverse(struct ForwardTreeList **pfList, int *byteNumber, int startingByte, struct ForwardTreeList **currentFwdTree) {

    char ch;

    int result = loadWhiteSpacesAndComments(byteNumber);
    if (result == ERROR) {

        ch = getchar();
        (*byteNumber)++;
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }

    ch = getchar();
    (*byteNumber)++;

    if (isDigit(ch)) {

        ungetc(ch, stdin);
        (*byteNumber)--;

        const char* num = loadNumber(byteNumber);

        if (num == NULL) {
            ch = getchar();
            errorInputOrEof((*pfList), ch, (*byteNumber));
        }

        getReverse(pfList, num, startingByte, (*currentFwdTree));

        free((void *) num);
    }

    else
        errorInputOrEof((*pfList), ch, (*byteNumber));

}

/** @brief Wczytuję dalszą część komendy dodawania przekierowania i wykonuję ją.
 * Funkcja wczytuję dalszą część komendy dodawania, po czym wykonuje to dodanie.
 * W razie jakichkolwiek błędów składniowych, bądź wykonania komendy kończy działanie programu i wypisuje stosowny błąd.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] byteNumber - wskaźnik na licznik wczytanych znaków;
 * @param[in] startingByte - numer pierwszego znaku wczytanego operatora;
 * @param[in,out] currentFwdTree - adres wskaźnika na aktualną bazę przekierowań;
 * @param[in] num1 - wskaźnik na numer, z którego jest przekierowanie.
 */
static void tryAddForward(struct ForwardTreeList **pfList, int *byteNumber, int startingByte, struct ForwardTreeList **currentFwdTree, const char *num1) {

    char ch;
    int result;

    result = loadWhiteSpacesAndComments(byteNumber);
    if (result == ERROR) {

        ch = getchar();
        (*byteNumber)++;
        free((void *) num1);
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }

    ch = getchar();
    (*byteNumber)++;

    if (isDigit(ch)) {

        ungetc(ch, stdin);
        (*byteNumber)--;

        const char* num2 = loadNumber(byteNumber);

        if (num2 == NULL) {
            ch = getchar();
            errorInputOrEof((*pfList), ch, (*byteNumber));
        }

        addForward(pfList, num1, num2, startingByte, (*currentFwdTree));
        free((void *) num1);
        free((void *) num2);
    }

    else {

        free((void *) num1);
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }
}

/** @brief Wczytuję dalszą część komendy wypisania lub dodania przekierowania i wykonuję ją.
 * Funkcja Wczytuję dalszą część komendy potem w trakcie działanie determinuję z którą komendą ma do czynienia
 * (dodawanie czy wypisanie przekierowania) i wczytuję jej dalszą część.
 * W przypadku jakichkolwiek błędów składniowych, bądź wykonania kończy działanie programu i wypisuje stosowny błąd.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] byteNumber - wskaźnik na licznik wczytanych znaków;
 * @param[in,out] currentFwdTree - adres wskaźnika na aktualną bazę przekierowań.
 */
static void tryGetOrAddForward(struct ForwardTreeList **pfList, int *byteNumber, struct ForwardTreeList **currentFwdTree) {

    char ch;
    int result;
    int startingByte;
    const char *num1 = loadNumber(byteNumber);

    if (num1 == NULL) {
        ch = getchar();
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }

    result = loadWhiteSpacesAndComments(byteNumber);
    if (result == ERROR) {

        ch = getchar();
        (*byteNumber)++;
        free((void *) num1);
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }

    ch = getchar();
    (*byteNumber)++;

    switch (ch) {

        case '?':

            startingByte = (*byteNumber);
            getForward(pfList, num1, startingByte, (*currentFwdTree));
            free((void *) num1);
            break;

        case '>':

            startingByte = (*byteNumber);
            tryAddForward(pfList, byteNumber, startingByte, currentFwdTree, num1);
            break;

        default:

            free((void *) num1);
            errorInputOrEof((*pfList), ch, (*byteNumber));
            break;
    }
}

/** @brief Wczytuję dalszą część komendy wypisania liczby nietrywialnych numerów.
 * Funkcja wczytuję dalszą część komendy wypisania liczby nietrywialnych numerów po czym wykonuję ją.
 * W razie jakichkolwiek błędów składniowych, bądź wykonania wypisuję stosowny komunikat i kończy działanie programu.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] byteNumber - wskaźnik na licznik wczytanych znaków;
 * @param[in] startingByte - numer pierwszego znaku wczytanego operatora;
 * @param[in,out] currentFwdTree - adres wskaźnika na aktualną bazę przekierowań.
 */
void tryCountNonTrivialCommand(struct ForwardTreeList **pfList, int *byteNumber, int startingByte, struct ForwardTreeList **currentFwdTree) {

    char ch;

    int result = loadWhiteSpacesAndComments(byteNumber);
    if (result == ERROR) {

        ch = getchar();
        (*byteNumber)++;
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }

    ch = getchar();
    (*byteNumber)++;

    if (isDigit(ch)) {

        ungetc(ch, stdin);
        (*byteNumber)--;

        const char* num = loadNumber(byteNumber);

        if (num == NULL) {
            ch = getchar();
            errorInputOrEof((*pfList), ch, (*byteNumber));
        }

        getNonTrivialCount(pfList, num, startingByte, (*currentFwdTree));

        free((void *) num);
    }

    else
        errorInputOrEof((*pfList), ch, (*byteNumber));
}

/** @brief Wczytuję komendę i wykonuje ją.
 * Funkcja wczytuję pojedynczą komendę po czym wykonuje ją.
 * W przypadku jakichkolwiek błędów składniowych, bądź wykonania kończy działanie programu i wypisuje stosowny błąd.
 * @param[in,out] pfList - adres wskaźnika na listę baz przekierowań;
 * @param[in,out] byteNumber - wskaźnik na licznik wczytanych znaków;
 * @param[in,out] currentFwdTree - adres wskaźnika na aktualną bazę przekierowań.
 */
static void loadAndExecuteCommand(struct ForwardTreeList **pfList, int *byteNumber, struct ForwardTreeList **currentFwdTree) {

    char ch;

    int result = loadWhiteSpacesAndComments(byteNumber);

    if (result == ERROR) {

        ch = getchar();
        (*byteNumber)++;
        errorInputOrEof((*pfList), ch, (*byteNumber));
    }

    ch = getchar();
    (*byteNumber)++;

    switch (ch) {

        case 'N':
            tryNewCommand(pfList, byteNumber, (*byteNumber), currentFwdTree);
            break;

        case 'D':
            tryDelCommand(pfList, byteNumber, (*byteNumber), currentFwdTree);
            break;

        case '?':
            tryReverse(pfList, byteNumber, (*byteNumber), currentFwdTree);
            break;

        case '@':
            tryCountNonTrivialCommand(pfList, byteNumber, (*byteNumber), currentFwdTree);
            break;

        default:
            if (isDigit(ch)) {

                ungetc(ch, stdin);
                (*byteNumber)--;
                tryGetOrAddForward(pfList, byteNumber, currentFwdTree);
            }

            else if (ch != EOF)
                errorInputOrEof((*pfList), ch, (*byteNumber));
    }
}

/** @brief Wczytuję pojedynczo wszystkie komendy z wejścia i je wykonuje.
 * Funkcja wczytuję wszystkie komendy z wejścia i po kolei je wykonuję.
 * W przypadku jakichkolwiek błędów składniowych, bądź wykonania kończy działanie programu i wypisuje stosowny błąd.
 * @return Wartość 0 w przypadku gdy nie wystąpił, żaden błąd,
 *         a wartość 1, gdy wystąpił błąd składniowy, bądź wykonania.
 */
int main() {

    int byteNumber = 0;

    struct ForwardTreeList *pfList = NULL;

    struct ForwardTreeList *currentBase = NULL;

    char ch = getchar();
    byteNumber++;

    while (ch != EOF) {

        ungetc(ch, stdin);
        byteNumber--;

        loadAndExecuteCommand(&(pfList), &byteNumber, &currentBase);

        ch = getchar();
        byteNumber++;
    }

    delFwdTreeList(pfList);

    return 0;
}