/** @file
 * Implementacja klasy przechowującej przekierowania numerów telefonicznych
 *
 * @author Aleksander Płocharski <ap394689@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 09.04.2018
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "phone_forward.h"

#define TO 0 /**<definiuje, że ma zostać dodane przekierowanie z danego numeru */
#define FROM 1 /**<definiuje, że ma zostać dodane przekierowanie na dany numeru */
#define NUMBER_OF_DIGITS 12 /**<liczba znaków uznawanych za cyfry */
#define NUMBER 0 /**<definiuję, że z listy ma być usunięty tylko konkretny numer */
#define PREFIX 1 /**<definiuję, że listy mają być usunięte wszystkie elementy o danym prefiksie */

struct PhoneForward * phfwdNew(void) {

    struct PhoneForward *pf = malloc(sizeof(struct PhoneForward));

    if (pf != NULL) {
        pf->fwdTo = NULL;
        for (int i = 0; i < NUMBER_OF_DIGITS; i++) {
            pf->children[i] = NULL;
        }

        pf->fwdFrom = NULL;
    }

    return pf;
}

void phnumDelete(struct PhoneNumbers const *pnum) {

    const struct PhoneNumbers *list = pnum;

    while (list != NULL) {
        const struct PhoneNumbers *tmp = list;
        list = list->next;

        if (tmp->number != NULL)
            free(tmp->number);

        free((void *) tmp);
    }
}

/** @brief Usuwa z listy wszystkie elementy zawierające numer o podanym prefiksie.
 * @param[in,out] pnum - adres wskaźnika na listę, z której usuwamy;
 * @param[in,out] prefix - wskaźnik na napis z jakim element ma być usunięty z listy.
 */
static void deletePrefixFromList(struct PhoneNumbers **pnum, const char* prefix) {

    if ((*pnum) != NULL) {

        size_t prefixLen = strlen(prefix);

        while ((*pnum) != NULL && strncmp((*pnum)->number, prefix, prefixLen) == 0) {

            struct PhoneNumbers *tmp = (*pnum)->next;
            if((*pnum)->number != NULL)
                free((*pnum)->number);
            free(*pnum);
            (*pnum) = tmp;
        }

        if ((*pnum) != NULL) {

            struct PhoneNumbers *tmp = (*pnum)->next;
            struct PhoneNumbers *prev = (*pnum);

            while (tmp != NULL) {

                while (tmp != NULL && strncmp(tmp->number, prefix, prefixLen) != 0) {
                    tmp = tmp->next;
                    prev = prev->next;
                }

                if (tmp != NULL) {

                    prev->next = tmp->next;
                    if (tmp->number != NULL)
                        free(tmp->number);
                    free(tmp);
                    tmp = prev->next;
                }
            }
        }
    }
}

/** @brief Usuwa z listy element o danym zapisanym napisie.
 * @param[in,out] pnum - adres wskaźnika na listę, z której usuwamy;
 * @param[in,out] num - wskaźnik na napis z jakim element ma być usunięty z listy.
 */
static void deleteNumFromList(struct PhoneNumbers **pnum, const char* num) {

    if ((*pnum) != NULL) {

        if (strcmp((*pnum)->number, num) == 0) {

            struct PhoneNumbers *tmp = (*pnum)->next;
            if((*pnum)->number != NULL)
                free((*pnum)->number);
            free(*pnum);
            (*pnum) = tmp;
        }

        else {
            struct PhoneNumbers *tmp = (*pnum)->next;
            struct PhoneNumbers *prev = (*pnum);

            while (tmp != NULL && strcmp(tmp->number, num) != 0) {
                tmp = tmp->next;
                prev = prev->next;
            }

            if (tmp != NULL) {

                prev->next = tmp->next;
                if (tmp->number != NULL)
                    free(tmp->number);
                free(tmp);
            }
        }
    }
}

void phfwdDelete(struct PhoneForward *pf) {

    if (pf != NULL) {

        for (int i = 0; i < NUMBER_OF_DIGITS; i++) {
            phfwdDelete(pf->children[i]);
        }

        if (pf->fwdTo != NULL) {
            free(pf->fwdTo);
            pf->fwdTo = NULL;
        }

        if (pf->fwdFrom != NULL) {
            phnumDelete(pf->fwdFrom);
            pf->fwdFrom = NULL;
        }

        free(pf);
    }
}

/** @brief Sprawdza czy znak jest cyfrą.
 * Sprawdza czy znak jest cyfrą według wymogów zadania.
 * @param ch - sprawdzany znak.
 * @return Wartość @p true jeśli znak jest cyfrą,
 *         wartość @p false jeśli znak nie jest cyfrą.
 */
static bool isDigit(char ch) {

    return (ch >= '0' && ch <= ';');
}

/** @brief Sprawdza czy napis jest numerem telefonu
 * Sprawdza czy podany w argumencie napis jest poprawnie zapisanym numerem telefonu
 * (według wymogów zadania).
 * @param[in] num - wskaźnik na sprawdzany napis.
 * @return Wartość @p true, jeśli napis jest poprawni zapisanym numerem.
 *         Wartość @p false, gdy napis nie reprezentuje numeru.
 */
static bool checkIfNumber(const char *num) {

    int i = 0;

    if (num == NULL || num[0] == '\0')
        return false;

    while (num[i] != '\0') {
        if (isDigit(num[i]) == 0)
            return false;
        i++;
    }

    return true;
}

/** @brief Konwertuje znak liczbowy na wartość.
 * @param[in] ch - konwertowany znak.
 * @return Wartość znaku.
 */
static int charDigitToInt(char ch) {

    return ch - '0';
}

/** @brief Tworzy element listy @ref PhoneNumbers.
 * @param[in] numLength - długość napisu, który ma się mieścić w elemencie.
 * @return Wskaźnik na nowo utworzony element.
 */
static struct PhoneNumbers *phnumNew(size_t numLength) {

    struct PhoneNumbers *pnum = malloc(sizeof(struct PhoneNumbers));

    if (pnum == NULL)
        return pnum;

    pnum->next = NULL;
    pnum->number = malloc(sizeof(char) * (numLength + 1));

    if (pnum->number == NULL) {
        free(pnum);
        return NULL;
    }

    return pnum;
}

/** @brief Tworzy pusty ciąg numerów telefonów.
 * @return Wskaźnik na powstały ciąg.
 */
static struct PhoneNumbers *emptyPhnum() {

    struct PhoneNumbers *empty;
    empty = phnumNew(0);

    if (empty != NULL)
        empty->number[0] = '\0';

    return empty;
}

/** @brief Sprawdza czy węzeł jest pusty.
 * Sprawdza, czy wszystkie pola w danym węźle są ustawione na NULL.
 * @param[in] pf - wskaźnik na sprawdzany węzeł.
 * @return Wartość @p true jeśli węzeł jest pusty, a @p false jeśli nie.
 */
static bool isNodeEmpty(struct PhoneForward *pf) {

    if (pf->fwdTo != NULL)
        return false;

    if (pf->fwdFrom != NULL)
        return false;

    for (int i = 0; i < NUMBER_OF_DIGITS; i++) {

        if (pf->children[i] != NULL)
            return false;
    }

    return true;
}

/** @brief Usuwa odpowiedni prefiks z listy przekierowujących się na drugi podany prefiks.
 * Znajduje, w drzewie prefiks wskazywany przez @p num i usuwa z jego list prefiksów,
 * które się na niego przekierowują element zawierający napis wskazywany przez @p numDel.
 * @param[in,out] pf - wskaźnik na drzewo przekierowań;
 * @param[in] num - wskaźnik na napis reprezentujący prefiks z którego usuwamy;
 * @param[in] numDel - wskaźnik na prefiks, który ma być usunięty z listy prefiksów, które przekierowują się na num
 * @param[in] currentDepth - aktualna głębokość w drzewie;
 * @param[in] length - długość prefiksu num;
 * @param[in] version - określa czy z listy ma być usunięty konkretny element o danym numerze, czy wszystkie z takim prefiksem.
 * @return Wartość @p true jeżeli węzeł wskazywany przez @p pf jest pusty po wykonaniu funkcji,
 *         wartość false jeżeli nie będzie pusty.
 */
static bool phfwdRemoveRecFrom(struct PhoneForward *pf, char const *num, char const *numDel, size_t currentDepth, size_t length, int version) {

    if (pf != NULL) {

        if (currentDepth == length) {
            if (version == PREFIX)
                deletePrefixFromList(&(pf->fwdFrom), numDel);
            if (version == NUMBER)
                deleteNumFromList(&(pf->fwdFrom), numDel);
            return isNodeEmpty(pf);
        }

        else {

            int digit = charDigitToInt(num[currentDepth]);

            if (phfwdRemoveRecFrom(pf->children[digit], num, numDel, currentDepth + 1, length, version) == true) {

                phfwdDelete(pf->children[digit]);
                pf->children[digit] = NULL;
            }
                return isNodeEmpty(pf);
        }
    }
    return false;
}

/** @brief Dodaje element o danym numerze do listy podanego węzła.
 * @param[in,out] pf - wskaźnik na węzeł, do którego dodajemy;
 * @param[in] num - numer, który ma zawierać dodawany element.
 * @return Wartość @p true jeśli udało się dodać element,
 *         wartość @p false w przeciwnym razie.
 */
static bool addToFromList(struct PhoneForward *pf, const char *num) {

    struct PhoneNumbers *number = phnumNew(strlen(num));

    if (number == NULL)
        return false;

    strcpy(number->number, num);
    number->next = pf->fwdFrom;
    pf->fwdFrom = number;

    return true;
}

/** @brief Dodaje przekierowanie o podanym numerze do węzła
 * Dodaje do węzła o prefiksie wskazywanym przez @p num1
 * przekierowanie o numerze wskazywanym przez @p num2.
 * Jeżeli przekierowanie już było dodane do węzła, zastępuje je.
 * @param[in,out] pfRoot - wskaźnik na drzewo przekierowań;
 * @param[in,out] pf - wskaźnik na obsługiwany węzeł;
 * @param[in] num1 - wskaźnik na prefiks węzła;
 * @param[in] num2 - wskaźnik na prefiks dodawany.
 * @return Wartość @p true jeżeli dodanie się powiodło,
 *         wartość @p false w przeciwnym razie.
 */
static bool addForward(struct PhoneForward *pfRoot, struct PhoneForward *pf, char const *num1, char const *num2) {

    if (pf->fwdTo != NULL) {
        phfwdRemoveRecFrom(pfRoot, pf->fwdTo, num1, 0, strlen(pf->fwdTo), NUMBER);
        free(pf->fwdTo);
    }

    pf->fwdTo = malloc(sizeof(char) * (strlen(num2) + 1));

    if (pf->fwdTo == NULL)
        return false;

    strcpy(pf->fwdTo, num2);

    return true;
}

/** @brief Dodaje przekierowanie z węzła, bądź do jego listy przekierowań, które na niego przechodzą.
 * Funkcja znajduje węzeł reprezentujący prefiks wskazywany przez @p num1.
 * Następnie w zależności o parametru version dodaje prefiks wskazywany przez @p num2 jako
 * numer, na który przekierowuje się prefiks z węzła (version == TO) lub dodaje prefiks
 * wskazywany przez @p num2 do list prefiksów, które przekierowują się na węzeł (version == FROM).
 * @param[in,out] pf - wskaźnik na drzewo przekierowań, do którego dodajemy;
 * @param[in] num1 - wskaźnik na prefiks, do którego dodajemy;
 * @param[in] num2 - wskaźnik na prefiks, który dodajemy
 * @param[in] version - definiuje wersje działania funkcji.
 * @return Wartość @p true jeżeli dodanie powiodło się,
 *         wartość @p false w przeciwnym przypadku.
 */
bool phfwdAddHelper(struct PhoneForward *pf, char const *num1, char const *num2, int version) {

    struct PhoneForward *tmp = pf;
    size_t length = strlen(num1);
    int digit;

    for (size_t i = 0; i < length; i++) {

        digit = charDigitToInt(num1[i]);
        if (tmp->children[digit] == NULL) {

            tmp->children[digit] = phfwdNew();
            if (tmp->children[digit] == NULL)
                return false;
        }

        tmp = tmp->children[digit];
    }

    if (version == 1)
        return addToFromList(tmp, num2);

    else
        return addForward(pf, tmp, num1, num2);
}

bool phfwdAdd(struct PhoneForward *pf, char const *num1, char const *num2) {

    if (!checkIfNumber(num1) || !checkIfNumber(num2))
        return false;

    if (strcmp(num1, num2) == 0)
        return false;

    if (!phfwdAddHelper(pf, num1, num2, TO))
        return false;

    return phfwdAddHelper(pf, num2, num1, FROM);
}

/** @brief Usuwa wszystkie przekierowania z danego poddrzewa.
 * Przechodzi po drzewie i usuwa wszystkie znalezione przekierowania.
 * Gdy jakieś znajdzie to przed jego usunięciem wywołuje funkcję, która usunie dane przekierowanie
 * z listy węzła, na który jest przekierowanie.
 * @param[in,out] rootPf - wskaźnik na korzeń drzewa przekierowań;
 * @param[in,out] pf - wskaźnik na aktualnie obsługiwany węzeł;
 * @param[in] num - napis reprezentujący prefiks, z którym przekierowania są usuwane.
 * @return Wartość @p true jeżeli po wywołaniu funkcji dla synów aktualnego węzła jest on pusty.
 *         Wartość @p false jeżeli po takim wywołaniu aktualny węzeł nie jest pusty.
 */
static bool removeForwardsFromSubtree(struct PhoneForward *rootPf, struct PhoneForward *pf, const char *num) {

    if (pf == NULL) {

        return false;
    }

    else {

        if (pf->fwdTo != NULL) {
            phfwdRemoveRecFrom(rootPf, pf->fwdTo, num, 0, strlen(pf->fwdTo), PREFIX);
        }

        for (int i = 0; i < NUMBER_OF_DIGITS; i++) {
            if (removeForwardsFromSubtree(rootPf, pf->children[i], num) == true) {
                phfwdDelete(pf->children[i]);
                pf->children[i] = NULL;
            }
        }

        if (pf->fwdTo != NULL) {
            char *tmp = pf->fwdTo;
            free(tmp);
            pf->fwdTo = NULL;
        }

        return isNodeEmpty(pf);
    }
}

/** @brief Usuwa wszystkie przekierowania o podanym prefiks.
 * Funkcja znajduje w drzewie węzeł odpowiadający prefiksowi wskazywanemu przez @p num.
 * Następnie usuwa wszystkie przekierowania z węzłów z poddrzewa, którego korzeniem jest znalexiony węzęł
 * @param[in,out] rootPf - wskaźnik na korzeń drzewa przekierowań;
 * @param[in,out] pf - wskaźnik na obsługiwany aktualnie węzeł;
 * @param[in] num - wskaźnik na usuwany prefiks;
 * @param[in] currentDepth - głębokość obsługiwanego węzła w drzewie;
 * @param[in] length - długość usuwanego prefiksu.
 * @return Wartość @p true jeśli węzeł wskazywany przez @p pf jest pusty po wykonaniu na nim funkcji.
 *         Wartość @p false jeśli nie dalej nie będzie pusty.
 */
static bool phfwdRemoveRecTo(struct PhoneForward *rootPf, struct PhoneForward *pf, char const *num, size_t currentDepth, size_t length) {

    if (pf != NULL) {

        if (currentDepth == length) {

            return removeForwardsFromSubtree(rootPf, pf, num);
        }

        else {

            int digit = charDigitToInt(num[currentDepth]);

            if (phfwdRemoveRecTo(rootPf, pf->children[digit], num, currentDepth + 1, length) == true) {

                phfwdDelete(pf->children[digit]);
                pf->children[digit] = NULL;

                return isNodeEmpty(pf);
            }
        }
    }

    return false;
}

void phfwdRemove(struct PhoneForward *pf, char const *num) {

    if (checkIfNumber(num) == true) {

        size_t length = strlen(num);
        phfwdRemoveRecTo(pf, pf, num, 0, length);
    }
}

struct PhoneNumbers const * phfwdGet(struct PhoneForward *pf, char const *num) {

    struct PhoneNumbers *numbers = NULL;

    if (checkIfNumber(num) == false)
        return emptyPhnum();

    char *bestMatch = NULL;
    size_t bestMatchLength = 0;
    bool endOfBranch = false;
    size_t length = strlen(num);

    for (size_t i = 0; i < length && !endOfBranch; i++) {
        int digit = charDigitToInt(num[i]);

        if (pf->children[digit] == NULL)
            endOfBranch = true;

        else {

            if (pf->children[digit]->fwdTo != NULL) {
                bestMatch = pf->children[digit]->fwdTo;
                bestMatchLength = i + 1;
            }

            pf = pf->children[digit];
        }
    }

    if (bestMatch == NULL) {
        numbers = phnumNew(length);

        if(numbers == NULL)
            return NULL;

        strcpy(numbers->number, num);
        return numbers;
    }

    numbers = phnumNew(length -bestMatchLength + strlen(bestMatch));

    if(numbers == NULL)
        return NULL;

    strcpy(numbers->number, bestMatch);
    strcpy(numbers->number + strlen(bestMatch), num + bestMatchLength);

    return numbers;
}

/** @brief Dodaje element do listy leksykograficznie
 * Dodaje do listy posortowanej leksykograficznie element w odpowiednim miejscu zachowując posortowanie.
 * Jeżeli taki element znajduje się już w liście, zwalnia go.
 * @param[in,out] list - adres wskaźnika na listę, do której chcemy dodać element.
 * @param[in,out] element - element dodawany do listy.
 */
static void addToListLex(struct PhoneNumbers **list, struct PhoneNumbers *element) {

    if ((*list) == NULL)
        (*list) = element;

    else {
        int difference = strcmp((*list)->number, element->number);

        if (difference > 0) {
            element->next = (*list);
            (*list) = element;
        }

        else if (difference == 0) {
            free(element->number);
            free(element);
        }

        else if (strcmp((*list)->number, element->number) < 0) {

            struct PhoneNumbers *tmp = *list;
            while (tmp->next != NULL && strcmp(tmp->next->number, element->number) < 0)
                tmp = tmp->next;

            if (tmp->next == NULL) {
                struct PhoneNumbers *tmp2 = tmp->next;
                tmp->next = element;
                element->next = tmp2;
            }

            else if(strcmp(tmp->next->number, element->number) == 0) {
                free(element->number);
                free(element);
            }

            else {
                struct PhoneNumbers *tmp2 = tmp->next;
                tmp->next = element;
                element->next = tmp2;
            }
        }
    }
}


struct PhoneNumbers const * phfwdReverse(struct PhoneForward *pf, char const *num) {

    if(checkIfNumber(num) == false)
        return emptyPhnum();

    struct PhoneNumbers *list = NULL;
    bool endOfBranch = false;
    struct PhoneForward *tmp = pf;
    size_t length = strlen(num);

    list = phnumNew(length + 1);
    strcpy(list->number, num);

    for(size_t i = 0; i < length && !endOfBranch; i++) {

        int index = charDigitToInt(num[i]);

        if (tmp->children[index] == NULL)
            endOfBranch = true;

        else {
            tmp = tmp->children[index];

            struct PhoneNumbers *nodeList = tmp->fwdFrom;

            while (nodeList != NULL) {

                struct PhoneNumbers *newNumber = phnumNew(length - i + 1 + strlen(nodeList->number) + 1);
                strcpy(newNumber->number, nodeList->number);
                strcpy((newNumber->number) + strlen(nodeList->number), num + (i +1));
                addToListLex(&list, newNumber);
                nodeList = nodeList->next;
            }
        }
    }

    return list;
}

char const * phnumGet(struct PhoneNumbers const *pnum, size_t idx) {

    if (pnum != NULL && pnum->number[0] == '\0')
        return NULL;

    char *number = NULL;
    const struct PhoneNumbers *list = pnum;

    for (size_t i = 0; i < idx && list != NULL; i++) {
        list = list->next;
    }

    if (list != NULL)
        number = list->number;

    return number;
}

/** @brief Upraszcza napis do tablicy mówiącej jakie cyfry zawiera
 * @param[in] set - wskaźnik na upraszczany napis;
 * @param[in,out] simplifiedSet - tablica, która będzie mówić jakie cyfry zawiera napis.
 */
static void getSimplifiedSet(char const *set, bool *simplifiedSet) {

    int i = 0;
    int numberOfUniqueDigits = 0;
    while (set[i] != '\0' && numberOfUniqueDigits < NUMBER_OF_DIGITS) {

        if (isDigit(set[i]) && simplifiedSet[charDigitToInt(set[i])] == false) {
            simplifiedSet[charDigitToInt(set[i])] = true;
            numberOfUniqueDigits++;
        }

        i++;
    }
}

/** @brief Wykonuję operację potęgowania i zwraca wynik.
 * @param[in] base - liczba podnoszona do potęgi;
 * @param[in] exponent - potęga, do której podnosimy liczbę.
 * @return Wynik potęgowania
 */
static size_t myPow(size_t base, size_t exponent) {

    size_t result = 1;

    while (exponent > 0) {

        if (exponent & 1) {

            result = (size_t) (result * base);
            exponent--;
        }

        else {

            exponent = exponent >> 1;
            base = (size_t)(base * base);
        }
    }

    return result;
}

/** @brief Rekurencyjnie zlicza numery nietrywialne o podanej długości i zawierające tylko cyfry z podanego zbioru.
 * @param[in] pf - wskaźnik na obsługiwany węzeł;
 * @param[in] depth - aktualna głębokość w drzewie;
 * @param[in] len - długość zliczanych numerów;
 * @param[in] setSize - ilość unikalnych cyfr w zbiorze;
 * @param[in] simplifiedSet - tablica mówiąca jakie cyfry są zawarte w zbiorze;
 * @param[in,out] counter - licznik numerów nietrywialnych.
 */
static void countNonTrivialRec(struct PhoneForward *pf, size_t depth, size_t len, size_t setSize, bool *simplifiedSet, size_t *counter) {

    if (pf != NULL && depth <= len) {

        if (pf->fwdFrom != NULL)
            (*counter) = (size_t)((*counter) + myPow(setSize, len - depth));

        else {

            for (int i = 0; i < NUMBER_OF_DIGITS; i++) {

                if (simplifiedSet[i] == true)
                    countNonTrivialRec(pf->children[i], depth + 1, len, setSize, simplifiedSet, counter);
            }
        }
    }

}

size_t phfwdNonTrivialCount(struct PhoneForward *pf, char const *set, size_t len) {

    if (pf == NULL || set == NULL || len == 0 || set[0] == '\0')
        return 0;

    bool simplifiedSet[NUMBER_OF_DIGITS];

    memset(simplifiedSet, 0, NUMBER_OF_DIGITS);

    getSimplifiedSet(set, simplifiedSet);

    size_t setSize = 0;

    for (int i = 0; i < NUMBER_OF_DIGITS; i++) {

        if (simplifiedSet[i] == true)
            setSize++;
    }

    if (setSize == 0)
        return 0;

    size_t counter = 0;

    countNonTrivialRec(pf, 0, len, setSize, simplifiedSet, &counter);

    return counter;
}
