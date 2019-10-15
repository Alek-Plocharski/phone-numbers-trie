/** @file
 * Interfejs klasy przechowującej przekierowania numerów telefonicznych
 *
 * @author Aleksander Płocharski <ap394689@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 09.04.2018
 */

#ifndef __PHONE_FORWARD_H__
#define __PHONE_FORWARD_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

/** @brief Struktura przechowująca przekierowania numerów telefonów.
 * Struktura przechowuję przekierowania numerów w drzewie.
 * Każdy węzeł reprezentuję jeden prefiks i posiada dziesięciu synów.
 * Jeżeli syn nie jest NULL'em reprezentuję on ten sam prefiks przedłużony o liczbę
 * zależną od jego pozycji w tablicy synów.
 * W węźle przechowywane jest prefiks, na który przekierowywany jest dany numer,
 * ale także lista prefiksów, które przekierowują się na ten numer.
 */
struct PhoneForward{

    struct PhoneForward *children[12]; /**< wskaźnik na poddrzewa reprezentujące kolejną liczbę w prefiksie (0-9) */
    char *fwdTo; /**< wskaźnik na prefiks na który przekierowywany jest węzeł */
    struct PhoneNumbers *fwdFrom; /**< wskaźnik na listę prefiksów, które przekierowują się na węzeł */
};

/** @brief Struktura przechowująca ciąg numerów telefonów.
 */
struct PhoneNumbers {

    struct PhoneNumbers *next; /**< wskaźnik na następny element w liście */
    char *number; /**< wskaźnik na napis reprezentujący numer telefonu */

};

/** @brief Tworzy nową strukturę.
 * Tworzy nową strukturę niezawierającą żadnych przekierowań.
 * @return Wskaźnik na utworzoną strukturę lub NULL, gdy nie udało się
 *         zaalokować pamięci.
 */
struct PhoneForward * phfwdNew(void);

/** @brief Usuwa strukturę.
 * Usuwa strukturę wskazywaną przez @p pf. Nic nie robi, jeśli wskaźnik ten ma
 * wartość NULL.
 * @param[in] pf – wskaźnik na usuwaną strukturę.
 */
void phfwdDelete(struct PhoneForward *pf);

/** @brief Dodaje przekierowanie.
 * Dodaje przekierowanie wszystkich numerów mających prefiks @p num1, na numery,
 * w których ten prefiks zamieniono odpowiednio na prefiks @p num2. Każdy numer
 * jest swoim własnym prefiksem. Jeśli wcześniej zostało dodane przekierowanie
 * z takim samym parametrem @p num1, to jest ono zastępowane.
 * @param[in] pf   – wskaźnik na strukturę przechowującą przekierowania numerów;
 * @param[in] num1 – wskaźnik na napis reprezentujący prefiks numerów
 *                   przekierowywanych;
 * @param[in] num2 – wskaźnik na napis reprezentujący prefiks numerów, na które
 *                   jest wykonywane przekierowanie.
 * @return Wartość @p true, jeśli przekierowanie zostało dodane.
 *         Wartość @p false, jeśli wystąpił błąd, np. podany napis nie
 *         reprezentuje numeru, oba podane numery są identyczne lub nie udało
 *         się zaalokować pamięci.
 */
bool phfwdAdd(struct PhoneForward *pf, char const *num1, char const *num2);

/** @brief Usuwa przekierowania.
 * Usuwa wszystkie przekierowania, w których parametr @p num jest prefiksem
 * parametru @p num1 użytego przy dodawaniu. Jeśli nie ma takich przekierowań
 * lub napis nie reprezentuje numeru, nic nie robi.
 *
 * @param[in] pf  – wskaźnik na strukturę przechowującą przekierowania numerów;
 * @param[in] num – wskaźnik na napis reprezentujący prefiks numerów.
 */
void phfwdRemove(struct PhoneForward *pf, char const *num);

/** @brief Wyznacza przekierowanie numeru.
 * Wyznacza przekierowanie podanego numeru. Szuka najdłuższego pasującego
 * prefiksu. Wynikiem jest co najwyżej jeden numer. Jeśli dany numer nie został
 * przekierowany, to wynikiem jest ten numer. Jeśli podany napis nie
 * reprezentuje numeru, wynikiem jest pusty ciąg. Alokuje strukturę
 * @p PhoneNumbers,która musi być zwolniona za pomocą funkcji @ref phnumDelete.
 * @param[in] pf  – wskaźnik na strukturę przechowującą przekierowania numerów;
 * @param[in] num – wskaźnik na napis reprezentujący numer.
 * @return Wskaźnik na strukturę przechowującą ciąg numerów.
 */
struct PhoneNumbers const * phfwdGet(struct PhoneForward *pf, char const *num);

/** @brief Wyznacza przekierowania na dany numer.
 * Wyznacza wszystkie przekierowania na podany numer. Wynikowy ciąg zawiera też
 * dany numer. Wynikowe numery są posortowane leksykograficznie i nie mogą się
 * powtarzać. Jeśli podany napis nie reprezentuje numeru, wynikiem jest pusty
 * ciąg. Alokuje strukturę @p PhoneNumbers, która musi być zwolniona za pomocą
 * funkcji @ref phnumDelete.
 * @param[in] pf  – wskaźnik na strukturę przechowującą przekierowania numerów;
 * @param[in] num – wskaźnik na napis reprezentujący numer.
 * @return Wskaźnik na strukturę przechowującą ciąg numerów.
 */
struct PhoneNumbers const * phfwdReverse(struct PhoneForward *pf, char const *num);

/** @brief Usuwa strukturę.
 * Usuwa strukturę wskazywaną przez @p pnum. Nic nie robi, jeśli wskaźnik ten ma
 * wartość NULL.
 * @param[in] pnum – wskaźnik na usuwaną strukturę.
 */
void phnumDelete(struct PhoneNumbers const *pnum);

/** @brief Udostępnia numer.
 * Udostępnia wskaźnik na napis reprezentujący numer. Napisy są indeksowane
 * kolejno od zera.
 * @param[in] pnum – wskaźnik na strukturę przechowującą ciąg napisów;
 * @param[in] idx  – indeks napisu.
 * @return Wskaźnik na napis. Wartość NULL, jeśli indeks ma za dużą wartość.
 */
char const * phnumGet(struct PhoneNumbers const *pnum, size_t idx);

/** @brief Oblicza liczbę nietrywialnych numerów telefonów zawierających konkretne cyfry i o podanej długości.
 * Numerem nietrywialnym nazywamy numer, dla którego w wyniku wywołania @ref phfwdReverse dla tego numeru
 * pojawia się numer inny niż on sam. Funkcja oblicza liczbę nietrywialnych numerów długości len zawierających tylko cyfry,
 * które znajdują się w napisie set. Wynik funkcji to liczba tych numerów modulo dwa do potęgi liczba bitów reprezentacji typu size_t.
 * Jeśli wskaźnik pf ma wartość NULL, set ma wartość NULL, set jest pusty,set nie zawiera żadnej cyfry
 * lub parametr len jest równy zeru, wynikiem jest zero.
 * @param[in] pf  - wskaźnik na strukturę przechowującą przekierowania numerów;
 * @param[in] set - wskaźnik na napis reprezentujący zbiór cyfr;
 * @param[in] len - długość wyznaczanych numerów.
 * @return Ilość numerów wyspecyfikowanych w opisie funkcji module dwa do potęgi liczby bitów reprezentacji typu size_t.
 */
size_t phfwdNonTrivialCount(struct PhoneForward *pf, char const *set, size_t len);

#endif /* __PHONE_FORWARD_H__ */
