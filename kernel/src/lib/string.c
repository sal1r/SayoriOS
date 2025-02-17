/**
 * @file lib/string.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Функции для работы со строками
 * @version 0.3.3
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023 
 */
#include <kernel.h>

bool isalnum(char c){
	return  (c >= '0' && c <= '9') ||
			(c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ;
}

/**
 * @brief Проверяет, является ли символ формата UTF-8
 *
 * @param char с - Символ
 *
 * @return bool - true если да
 */
bool isUTF(char c){
    if (c == -47 || c == -48){
        return true;
    }
    return false;
}

/**
 * @brief Проверяет, является ли специальным символом
 *
 * @param char с - Символ
 *
 * @return bool - true если да
 */
bool isSymbol(char c){
    if (c == -62 || c == -106 || c == -30){
        return true;
    }
    return false;
}
/**
 * @brief Возращает индекс символа
 *
 * @param char с - Символ
 * @param char с1 - Символ
 * @param char с2 - Символ
 *
 * @return uint32_t - индекс символа
 */
uint32_t SymConvert(char c,char c1,char c2){
    uint32_t s = 0;
    if (c == -62){
        s = 1000;
    } else if (c == -106){
        s = 2000+(((int) c1)* -1);
    } else if (c == -30){
        s = 3000+(((int) c1)* -1);
    }
    return (isSymbol(c)?((((int) c)* -1)+(((int) c1)* -1)+s):((int) c));
}
/**
 * @brief Возращает индекс символа
 *
 * @param char с - Символ
 * @param char с1 - Символ
 *
 * @return uint32_t - индекс символа
 */
uint32_t UTFConvert(char c,char c1){
    return (isUTF(c)?((((int) c)* -1)+(((int) c1)* -1)):((int) c));
}

/**
 * @brief Возращает длину строки
 *
 * @param char* str - Строка
 *
 * @return size_t - Длину символов
 */
size_t strlen(const char *str){
	ON_NULLPTR(str, {
		return 0;
	});

    size_t len = 0;
    while (str[len] != 0){
        len++;
    }
    return len;
}

/**
 * @brief Возращает длину строки с учетом UTF-8
 *
 * @param char* str - Строка
 *
 * @return size_t - Длину символов
 */
size_t mb_strlen(const char *str){
    size_t len = 0;
    size_t def = strlen(str);
    for(size_t i = 0; i < def;i++){
        if (isUTF(str[i])) continue;
        len++;
    }
    return len;
}

// Increments a counter until encounters char `find`
size_t struntil(const char* str, const char find) {
    int len = 0;

    while(str[len++] != find);

    return len;
}

/**
 * @brief Копирование непересекающихся массивов
 *
 * @param void* destination - Указатель на массив в который будут скопированы данные.
 * @param void* source - Указатель на массив источник копируемых данных.
 * @param size_t n - Количество байт для копирования
 */

#if USE_SSE
void sse_memcpy(void* restrict dest, const void* restrict src, size_t size) {
    size_t i;

    size_t alignedd = (size_t)dest & ~0xF;
    size_t aligneds = (size_t)src & ~0xF;

    qemu_log("Difference dest: %x", (size_t)dest - alignedd);
    qemu_log("Difference src: %x", (size_t)src - aligneds);

    __m128i* d = (__m128i*)dest;
    const __m128i* s = (__m128i*)src;
    const size_t num_elems = size / sizeof(__m128i);

    for (i = 0; i < num_elems; i++) {
        _mm_stream_si128(d, _mm_load_si128(s));
        d++;
        s++;
    }

    // Copy any remaining bytes using the regular memcpy
    size_t remaining_bytes = size % sizeof(__m128i);
    if (remaining_bytes > 0) {
        const char* s_char = (const char*)s;
        char* d_char = (char*)d;
        for (i = 0; i < remaining_bytes; i++) {
            *d_char = *s_char;
            d_char++;
            s_char++;
        }
    }
}
#endif

#define USE_ENCHANCED_MEMCPY 0

#if USE_ENCHANCED_MEMCPY == 1
void memcpy(void *restrict destination, const void *restrict source, size_t n){
    size_t *tmp_dest = (size_t *)destination;
    size_t *tmp_src = (size_t *)source;
    size_t len = n / sizeof(size_t);
    size_t i = 0;
    size_t tail = n & (sizeof(size_t) - 1);

    for (; i < len; i++) {
        *tmp_dest++ = *tmp_src++;
    }

    if(tail) {
        char *dest = (char *)destination;
        const char *src = (const char *)source;
    
        for(i = n - tail; i < n; i++) {
            dest[i] = src[i];
        }
    }
}
#else
void memcpy(void* destination, const void* source, size_t n) {
    char *dest = (char *)destination;
    const char *src = (const char*)source;

    while(n--)
        *dest++ = *src++;
}
#endif

/**
 * @brief Заполнение массива указанными символами
 *
 * @param void* ptr - Указатель на заполняемый массив
 * @param void* value - Код символа для заполнения
 * @param size_t size - Размер заполняемой части массива в байтах
 */
void* memset(void* ptr, int value, size_t num) {
    uint8_t* p = (uint8_t*)ptr;
    unsigned char val = (unsigned char)value;
 
    for (size_t i = 0; i < num; i++) {
        p[i] = val;
    }
 
    return ptr;
}

/**
 * @brief Копирование массивов (в том числе пересекающихся)
 *
 * @param void* dest - Указатель на массив в который будут скопированы данные.
 * @param void* src - Указатель на массив источник копируемых данных
 * @param size_t count - Количество байт для копирования
 */
void* memmove(void *dest, void *src, size_t count)
{
	void * ret = dest;
	if (dest <= src || (char*)dest >= ((char*)src + count))
	{
		while (count--)
		{
			*(char*)dest = *(char*)src;
			dest = (char*)dest + 1;
			src = (char*)src + 1;
		}
	}
	else
	{
		dest = (char*)dest + count - 1;
		src = (char*)src + count - 1;
		while (count--)
		{
			*(char*)dest = *(char*)src;
			dest = (char*)dest - 1;
			src = (char*)src - 1;
		}
	}
	return ret;
}

/**
 * @brief Сравнение строк
 *
 * @param char* s1 - Строка 1
 * @param char* s2 - Строка 2
 *
 * @return int - Возращает 0 если строки идентичны или разницу между ними
 */
int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
        ++s1;
        ++s2;
    }

    return (*s1 - *s2);
}

/**
 * @brief Сравнение строк
 *
 * @param char* s1 - Строка 1
 * @param char* s2 - Строка 2
 *
 * @return bool - Возращает true если строки идентичны
 */
bool strcmpn(const char *str1, const char *str2){
    return strcmp(str1, str2) == 0;
}

/**
 * @brief Копирование строк
 *
 * @param char* dest - Указатель на строку, в которую будут скопированы данные
 * @param char* src - Указатель на строку источник копируемых данных
 *
 * @return int - Функция возвращает указатель на строку, в которую скопированы данные.
 */
int strcpy(char* dest, const char* src){
    int i = 0;

    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }

    dest[i] = '\0';

    return i;
}

/**
 * @brief Сравнение массивов
 *
 * @param void* s1 - Указатель на строку
 * @param void* s2 - Указатель на строку
 * @param size_t n - Размер сравниваемой части массива в байтах.
 *
 * @return int - Возращает 0 если строки идентичны или разницу между ними
 */
int32_t memcmp(const char *s1, const char *s2, size_t n){
    unsigned char u1, u2;

    for (; n--; s1++, s2++){
        u1 = *(unsigned char *)s1;
        u2 = *(unsigned char *)s2;
        if (u1 != u2){
            return (u1 - u2);
        }
    }
    return 0;
}
/**
 * @brief ???
 *
 * @param char* str - ???
 * @param char c - ???
 *
 * @return size_t - ???
 */
size_t str_bksp(char *str, char c){
    size_t i = strlen(str);
    i--;
    while (i){
        i--;
        if (str[i] == c){
            str[i + 1] = 0;
            return 1;
        }
    }
    return 0;
}
/**
 * @brief ???
 *
 * @param char* s - ???
 * @param char* accept - ???
 *
 * @return char* - ???
 */
char *strpbrk(const char *s, const char *accept){
    while (*s != '\0'){
        const char *a = accept;
        while (*a != '\0'){
            if (*a++ == *s){
                return (char *)s;
            }
        }
        ++s;
    }
    return nullptr;
}

/**
 * @brief Определение максимальной длины участка строки, содержащего только указанные символы
 *
 * @param char* s - Указатель на строку, в которой ведется поиск
 * @param char* accept - Указатель на строку с набором символов, которые должны входить в участок строки str
 *
 * @return size_t - Длина начального участка строки, содержащая только символы, указанные в аргументе sym
 */
size_t strspn(const char *s, const char *accept){
    const char *p;
    const char *a;
    size_t count = 0;

    for (p = s; *p != '\0'; ++p){
        for (a = accept; *a != '\0'; ++a){
            if (*p == *a){
                break;
            }
        }
        if (*a == '\0'){
            return count;
        }
        else{
            ++count;
        }
    }
    return count;
}

/**
 * @brief Сравнение строк с ограничением количества сравниваемых символов
 *
 * @param char* s1 - Строка 1
 * @param char* s2 - Строка 2
 *
 * @return int - Возращает 0 если строки идентичны или разницу между ними
 */
int32_t strncmp(const char *s1, const char *s2, size_t num){
    for (size_t i = 0; i < num; i++){
        if (s1[i] != s2[i]){
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Разбиение строки на части по указанному разделителю
 *
 * @param char* s - Указатель на разбиваемую строку
 * @param char* delim - Указатель на строку, содержащую набор символов разделителей
 *
 * @return int - nullptr – если строку str невозможно разделить на части или указатель на первый символ выделенной части строки.
 */
char *strtok(char *s, const char *delim){
    static char *olds = nullptr;
    char *token;

    if (s == nullptr){
        s = olds;
    }

    s += strspn(s, delim);

    if (*s == '\0'){
        olds = s;
        return nullptr;
    }

    token = s;
    s = strpbrk(token, delim);
    if (s == nullptr){
        olds = token;
    }
    else{
        *s = '\0';
        olds = s + 1;
    }
    return token;
}

/**
 * @brief Копирование строк c ограничением длины
 *
 * @param char* dest - Указатель на строку, в которую будут скопированы данные
 * @param char* src - Указатель на строку источник копируемых данных
 * @param size_t n - Ограничение длинны копирования
 *
 * @return char* - Функция возвращает указатель на строку, в которую скопированы данные
 */
char *strncpy(char *dest, const char *src, size_t n){
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
    return dest;
}

/**
 * @brief Объединение строк
 *
 * @param char* s - Указатель на массив в который будет добавлена строка
 * @param char* t - Указатель на массив из которого будет скопирована строка
 *
 * @return char* - Функция возвращает указатель на массив, в который добавлена строка
 */
char *strcat(char *s, const char *t){
    strcpy(s + strlen(s), t);
    return s;
}

/**
 * @brief Вырезает и возвращает подстроку из строки
 *
 * @param char* dest - Указатель куда будет записана строка
 * @param char* source - Указатель на исходную строку
 * @param int source - Откуда копируем
 * @param size_t source - Количество копируемых строк
 */
void substr(char *dest, const char *source, int from, int length){
    strncpy(dest, source + from, length);
    dest[length] = 0;
}

/**
 * @brief Поиск первого вхождения символа в строку
 *
 * @param char* _s - Указатель на строку, в которой будет осуществляться поиск.
 * @param int _c - Код искомого символа
 *
 * @return char* - Указатель на искомый символ, если он найден в строке str, иначе nullptr.
 */
char *strchr(const char *_s, char _c){
    while (*_s != (char)_c){
        if (!*_s++){
            return 0;
        }
    }
    return (char *)_s;
}

/**
 * @brief Перевод строки в нижний регистр
 *
 * @param char* as - Указатель на строку.
 */
void strtolower(char* as){
	while(*as != 0)
	{
		if(*as >= 'A' && *as <= 'Z')
			*as += ('a' - 'A');
		as++;
	}
}

/**
 * @brief Перевод строки в верхний регистр
 *
 * @param char* as - Указатель на строку.
 */
void strtoupper(char* as){
	while(*as != 0)
	{
		if(*as >= 'a' && *as <= 'z')
			*as -= ('a' - 'A');
		as++;
	}
}

/**
 * @brief Проверяет, является ли строка числом
 *
 * @param char* c - Указатель на строку.
 *
 * @return bool - если строка является числом
 */
bool isNumber(char * c){
    for(uint32_t i = 0, len = strlen(c); i < len; i++){
        if ((uint32_t) c[i] >= 48 && (uint32_t) c[i] <= 57){
            continue;
        } else {
            return false;
        }
    }
    return true;
}

/**
 * @brief Превращает строку в число
 *
 * @param char[] s - Указатель на строку.
 *
 * @return uint32_t - Число
 */
uint32_t atoi(char s[]){
    int i, n;
    n = 0;
    for (i = 0; s[i] >= '0' && s[i] <= '9'; ++i)
        n = (n * 10) + (s[i] - '0');
    return n;
}

/**
 * @brief Выделение памяти при использовании кучи вместо стека
 *
 * @param nmemb - ???
 * @param size	- ???
 * @return void* - ???
 */
void* calloc(size_t nmemb, size_t size) {
	void* ptr = kmalloc(nmemb * size);
	if (!ptr) {
		return nullptr;
	}
	memset(ptr, 0, nmemb * size);
	return ptr;
}


/**
 * @brief Переворачивает строку задом наперед
 *
 * @param char* str - строка символов, которая должна быть обращена
 */
void strver(char *str) {
    char c;
    int32_t j = strlen(str) - 1;

    for (int32_t i = 0; i < j; i++) {
        c = str[i];
        str[i] = str[j];
        str[j] = c;
        j--;
    }
}

/**
 * @brief Конвертируем число в символы
 *
 * @param int32_t n - Число
 * @param char* buffer - символы
 * @return int32_t - Длина строки
 */
int32_t itoa(int32_t n, char *buffer) {
    char const digits[] = "0123456789";
    char* p = buffer;

    if (n < 0){
        *p++ = '-';
        n *= -1;
    }

    int s = n;

    do {
        ++p;
        s = s / 10;
    } while(s);

    *p = '\0';

    do {
        *--p = digits[n % 10];
        n = n / 10;
    } while(n);

    return strlen(buffer);
}

// Unsigned int to charptr
size_t itou(size_t n, char *buffer) {
    char const digits[] = "0123456789";
    char* p = buffer;

    size_t s = n;

    do {
        ++p;
        s = s / 10;
    } while(s);

    *p = '\0';

    do {
        *--p = digits[n % 10];
        n = n / 10;
    } while(n);

    return strlen(buffer);
}

size_t itoh(size_t i, char *buffer) {
	const unsigned char hex[16] = "0123456789ABCDEF";
    uint32_t n, d = 0x10000000;
    char* p = buffer;

    while ((i / d == 0) && (d >= 0x10)) {
        d /= 0x10;
    }
    n = i;

    while (d >= 0xF) {
         *p++ = hex[n / d];
        n = n % d;
        d /= 0x10;
    }

	*p++ = hex[n];
    *p = 0;

    return strlen(buffer);
}


int dcmpstr( const char *s1, const char *s2 )
{
    while ( *s1 && *s1 == *s2 ) ++s1, ++s2;

    return ( ( unsigned char )*s1 > ( unsigned char )*s2 ) -
           ( ( unsigned char )*s1 < ( unsigned char )*s2 );
}

char digit_count(size_t num) {
    if(num == 0)
        return 1;
    
    char count = 0;
    while(num > 0) {
        num /= 10;
        count++;
    }
    return count;
}

char hex_count(size_t num) {
    if(num == 0)
        return 1;
    
    char _ = 0;
    while(num > 0) {
        num /= 16;
        _++;
    }
    return _;
}

bool isdigit(char a) {
    return (a >= '0' && a <= '9');
}

bool isnumberstr(char* a) {
    while(*a) {
        if(!isdigit(*a)) {
            qemu_log("%c is not a digit!", *a);
            return false;
        }
        
        a++;
    }

    return true;
}

/**
 * @brief Посчитать количество символов `character` в строке `string` 
 *
 * @param const char* s1 - Строка
 * @param char character - Символ
 *
 * @return size_t - количество найденных символов в строке
 */
size_t strcount(const char* string, char character) {
    size_t count = 0;

    while(*string++) {
        if(*string == character)
            count++;
    }

    return count;
}