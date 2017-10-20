/* ’‚ «ANSIŒƒµµ */

#ifndef __LIB__JSON__H__
#define __LIB__JSON__H__

#include "cJSON.h"

#ifdef __cplusplus

class Lib_Json
{
public:
	~Lib_Json(void);

	bool loadChildObject(const char *name, bool fromChild = true);
	bool loadChildArray(const char *name, bool fromChild = true);
	bool loadChildObjectFromChildArrayByIndex(const int index, bool fromChild = true);

	void releaseChild(void);

	bool putValueString(const char *name, const char *buffer, bool fromChild = true);

	bool getValueBoolean(const char *name, bool fromChild = true);

	bool putValueNumber(const char *name, int value, bool fromChild = true);
	bool putValueNumber(const char *name, unsigned int value, bool fromChild = true);
	bool putValueNumber(const char *name, long long int value, bool fromChild = true);
	bool putValueNumber(const char *name, unsigned long long int value, bool fromChild = true);
	bool putValueNumber(const char *name, double value, bool fromChild = true);

	bool getValueString(const char *name, char *buffer, unsigned int size, bool fromChild = true);

	bool getValueNumber(const char *name, short *value, bool fromChild = true);
	bool getValueNumber(const char *name, unsigned short *value, bool fromChild = true);
	bool getValueNumber(const char *name, int *value, bool fromChild = true);
	bool getValueNumber(const char *name, unsigned int *value, bool fromChild = true);
	bool getValueNumber(const char *name, long long int *value, bool fromChild = true);
	bool getValueNumber(const char *name, unsigned long long int *value, bool fromChild = true);
	bool getValueNumber(const char *name, double *value, bool fromChild = true);

	bool getValueArray(const char *name, cJSON **value, bool fromChild = true);
	bool getValueObject(const char *name, cJSON **value, bool fromChild = true, bool dup = false);

	bool getValueNumberFromArray(int *value, unsigned int n, bool fromChild = true);
	bool getValueNumberFromArray(unsigned int *value, unsigned int n, bool fromChild = true);
	bool getValueNumberFromArray(long long int *value, unsigned int n, bool fromChild = true);
	bool getValueNumberFromArray(unsigned long long int *value, unsigned int n, bool fromChild = true);
	bool getValueNumberFromArray(double *value, unsigned int n, bool fromChild = true);

	bool getValueStringFromArray(char **value, unsigned int size, unsigned int n, bool fromChild = true);

	void dumpObject(bool fromChild = true);
	void dumpArray(void);

	int getLenght(void);

	bool saveFile(const char *path, bool useFormat = true);

	cJSON *cSJONDup(void);

//  *** for static function *********************************************************************

	static bool putValueString(cJSON *root, const char *name, const char *buffer);

	static bool putValueNumber(cJSON *root, const char *name, int value);
	static bool putValueNumber(cJSON *root, const char *name, unsigned int value);
	static bool putValueNumber(cJSON *root, const char *name, long long int value);
	static bool putValueNumber(cJSON *root, const char *name, unsigned long long int value);
	static bool putValueNumber(cJSON *root, const char *name, double value);

	static bool getValueString(cJSON *root, const char *name, char *buffer, unsigned int size);

	static bool getValueBoolean(cJSON *root, const char *name);

	static bool getValueNumber(cJSON *root, const char *name, short *value);
	static bool getValueNumber(cJSON *root, const char *name, unsigned short *value);
	static bool getValueNumber(cJSON *root, const char *name, int *value);
	static bool getValueNumber(cJSON *root, const char *name, unsigned int *value);
	static bool getValueNumber(cJSON *root, const char *name, long long int *value);
	static bool getValueNumber(cJSON *root, const char *name, unsigned long long int *value);
	static bool getValueNumber(cJSON *root, const char *name, double *value);

	static bool getValueArray(cJSON *root, const char *name, cJSON **value);
	static bool getValueObject(cJSON *root, const char *name, cJSON **value, bool dup = false);

	static bool getValueNumberFromArray(cJSON *root, int *value, unsigned int n);
	static bool getValueNumberFromArray(cJSON *root, unsigned int *value, unsigned int n);
	static bool getValueNumberFromArray(cJSON *root, long long int *value, unsigned int n);
	static bool getValueNumberFromArray(cJSON *root, unsigned long long int *value, unsigned int n);
	static bool getValueNumberFromArray(cJSON *root, double *value, unsigned int n);

	static bool getValueStringFromArray(cJSON *root, char **value, unsigned int size, unsigned int n);

	static void dumpAll(cJSON *root, bool useFormat = true);
	static int getLenght(cJSON *root);
	static cJSON *loadFile(const char *path);
	static bool saveFile(cJSON *root, const char *path, bool useFormat = true);
	static void freeAll(cJSON **root);

	static Lib_Json *getObject(void *root);
	static Lib_Json *getObject(cJSON *root);
	static Lib_Json *getObject(const char *buffer, int size);
	static Lib_Json *getObject(const char *path);

private:
	Lib_Json(void);

	cJSON *root;
	cJSON *child;
	cJSON *array;
};

#endif

#endif
