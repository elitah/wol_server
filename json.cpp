
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>

#include <queue>

#include "json.h"

#ifdef __cplusplus

#define GetObjectItemValueS(__type, __root, __t, __name, __code)		({ \
																			bool ret = false; \
																			cJSON *(__t) = cJSON_GetObjectItem((__root), (__name)); \
																			if(NULL != (__t) && (__type) == (__t)->type) \
																			{ \
																				__code; \
																				ret = true; \
																			} \
																			ret; \
																		})

#define GetObjectItemValueM(__type, __root, __t, __name, __code)		({ \
																			bool ret = false; \
																			cJSON *(__t) = cJSON_GetObjectItem((__root), (__name)); \
																			if(NULL != (__t) && (__type) == (__t)->type) \
																			{ \
																				__code; \
																			} \
																			ret; \
																		})

#define GetValueNumberFromObject(__root, __name, __value, __k)			({ \
																			bool ret = false; \
																			if(NULL != (__root)) \
																			{ \
																				typeof(__root) __t = cJSON_GetObjectItem((__root), (__name)); \
																				if(NULL != __t && cJSON_Number == __t->type) \
																				{ \
																					(__value)[0] = (typeof((__value)[0]))__t->__k; \
																					ret = true; \
																				} \
																			} \
																			ret; \
																		})

#define GetValueNumberToObject(__root, __name, __value)					({ \
																			bool ret = false; \
																			if(NULL != (__root)) \
																			{ \
																				typeof(__root) __t = cJSON_GetObjectItem((__root), (__name)); \
																				if(NULL != __t && cJSON_Number == __t->type) \
																				{ \
																					__t->valueint = (__value); \
																					__t->valuedouble = (__value); \
																					ret = true; \
																				} \
																			} \
																			ret; \
																		})

#define GetValueNumberFromArray(__root, __value, __n, __k)					({ \
																			bool ret = false; \
																			if(NULL != (__root)) \
																			{ \
																				typeof(__n) i = 0; \
																				typeof(__root) child = (__root)->child; \
																				for(i = 0; (__n) > i; i++) \
																				{ \
																					if(NULL == child || cJSON_Number != child->type) \
																					{ \
																						break; \
																					} \
																					(__value)[i] = (typeof((__value)[i]))child->__k; \
																					child = child->next; \
																				} \
																				if(i == (__n)) \
																				{ \
																					ret = true; \
																				} \
																			} \
																			ret; \
																		})

Lib_Json::Lib_Json(void)
	: root(NULL)
	, child(NULL)
	, array(NULL)
{
}

Lib_Json::~Lib_Json(void)
{
	child = NULL;
	array = NULL;

	if(NULL != root)
	{
		cJSON_Delete(root);
		root = NULL;
	}
}

bool Lib_Json::loadChildObject(const char *name, bool fromChild)
{
	cJSON *p = (true == fromChild) ? ((NULL != child) ? child : root) : root;

	if(NULL != p && cJSON_Object == p->type)
	{
		return Lib_Json::getValueObject(p, name, &child);
	}

	return false;
}

bool Lib_Json::loadChildArray(const char *name, bool fromChild)
{
	cJSON *p = (true == fromChild) ? ((NULL != child) ? child : root) : root;

	if(NULL != p && cJSON_Object == p->type)
	{
		return Lib_Json::getValueArray(p, name, &array);
	}

	return false;
}

bool Lib_Json::loadChildObjectFromChildArrayByIndex(const int index, bool fromChild)
{
	cJSON *p = (true == fromChild) ? ((NULL != array) ? array : root) : root;

	if(NULL != p && cJSON_Array == p->type)
	{
		cJSON *list = p->child;

		for(int i = 0; index > i && NULL != list; i++)
		{
			list = list->next;
		}

		if(NULL != list)
		{
			child = list;

			return true;
		}
	}

	return false;
}

void Lib_Json::releaseChild(void)
{
	child = NULL;
	array = NULL;
}

bool Lib_Json::putValueString(const char *name, const char *buffer, bool fromChild)
{
	return Lib_Json::putValueString((true == fromChild) ? ((NULL != child) ? child : root) : root, name, buffer);
}

bool Lib_Json::getValueBoolean(const char *name, bool fromChild)
{
	return Lib_Json::getValueBoolean((true == fromChild) ? ((NULL != child) ? child : root) : root, name);
}

bool Lib_Json::putValueNumber(const char *name, int value, bool fromChild)
{
	return Lib_Json::putValueNumber((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value);
}

bool Lib_Json::putValueNumber(const char *name, unsigned int value, bool fromChild)
{
	return Lib_Json::putValueNumber((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value);
}

bool Lib_Json::putValueNumber(const char *name, long long int value, bool fromChild)
{
	return Lib_Json::putValueNumber((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value);
}

bool Lib_Json::putValueNumber(const char *name, unsigned long long int value, bool fromChild)
{
	return Lib_Json::putValueNumber((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value);
}

bool Lib_Json::putValueNumber(const char *name, double value, bool fromChild)
{
	return Lib_Json::putValueNumber((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value);
}

bool Lib_Json::getValueString(const char *name, char *buffer, unsigned int size, bool fromChild)
{
	return Lib_Json::getValueString((true == fromChild) ? ((NULL != child) ? child : root) : root, name, buffer, size);
}

bool Lib_Json::getValueNumber(const char *name, short *value, bool fromChild)
{
	return Lib_Json::getValueNumber((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value);
}

bool Lib_Json::getValueNumber(const char *name, unsigned short *value, bool fromChild)
{
	return Lib_Json::getValueNumber((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value);
}

bool Lib_Json::getValueNumber(const char *name, int *value, bool fromChild)
{
	return Lib_Json::getValueNumber((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value);
}

bool Lib_Json::getValueNumber(const char *name, unsigned int *value, bool fromChild)
{
	return Lib_Json::getValueNumber((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value);
}

bool Lib_Json::getValueNumber(const char *name, long long int *value, bool fromChild)
{
	return Lib_Json::getValueNumber((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value);
}

bool Lib_Json::getValueNumber(const char *name, unsigned long long int *value, bool fromChild)
{
	return Lib_Json::getValueNumber((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value);
}

bool Lib_Json::getValueNumber(const char *name, double *value, bool fromChild)
{
	return Lib_Json::getValueNumber((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value);
}

bool Lib_Json::getValueArray(const char *name, cJSON **value, bool fromChild)
{
	if(NULL == name && NULL != value)
	{
		*value = array;

		return NULL != array ? true : false;
	}

	return Lib_Json::getValueArray((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value);
}

bool Lib_Json::getValueObject(const char *name, cJSON **value, bool fromChild, bool dup)
{
	if(NULL == name && NULL != value)
	{
		*value = child;

		return NULL != child ? true : false;
	}

	return Lib_Json::getValueObject((true == fromChild) ? ((NULL != child) ? child : root) : root, name, value, dup);
}

bool Lib_Json::getValueNumberFromArray(int *value, unsigned int n, bool fromChild)
{
	return Lib_Json::getValueNumberFromArray((true == fromChild) ? ((NULL != array) ? array : root) : root, value, n);
}

bool Lib_Json::getValueNumberFromArray(unsigned int *value, unsigned int n, bool fromChild)
{
	return Lib_Json::getValueNumberFromArray((true == fromChild) ? ((NULL != array) ? array : root) : root, value, n);
}

bool Lib_Json::getValueNumberFromArray(long long int *value, unsigned int n, bool fromChild)
{
	return Lib_Json::getValueNumberFromArray((true == fromChild) ? ((NULL != array) ? array : root) : root, value, n);
}

bool Lib_Json::getValueNumberFromArray(unsigned long long int *value, unsigned int n, bool fromChild)
{
	return Lib_Json::getValueNumberFromArray((true == fromChild) ? ((NULL != array) ? array : root) : root, value, n);
}

bool Lib_Json::getValueNumberFromArray(double *value, unsigned int n, bool fromChild)
{
	return Lib_Json::getValueNumberFromArray((true == fromChild) ? ((NULL != array) ? array : root) : root, value, n);
}

bool Lib_Json::getValueStringFromArray(char **value, unsigned int size, unsigned int n, bool fromChild)
{
	return Lib_Json::getValueStringFromArray((true == fromChild) ? ((NULL != array) ? array : root) : root, value, size, n);
}

void Lib_Json::dumpObject(bool fromChild)
{
	Lib_Json::dumpAll((true == fromChild) ? ((NULL != child) ? child : root) : root);
}

void Lib_Json::dumpArray(void)
{
	if(NULL != array)
	{
		Lib_Json::dumpAll(array);
	}
}

int Lib_Json::getLenght(void)
{
	int len = Lib_Json::getLenght(root);

	printf("Lib_Json::getLenght: %d\n", len);

	return len;
}

bool Lib_Json::saveFile(const char *path, bool useFormat)
{
	return Lib_Json::saveFile(root, path, useFormat);
}

cJSON *Lib_Json::cSJONDup(void)
{
	return cJSON_Duplicate(root, 1);
}

//############# for static ######################################################################################################333

bool Lib_Json::putValueString(cJSON *root, const char *name, const char *buffer)
{
	if(NULL != root && NULL != name && 0 < strlen(name) && NULL != buffer)
	{
		return GetObjectItemValueM(cJSON_String, root, pTemp, name, {
			free(pTemp->valuestring);
			pTemp->valuestring = NULL;
			pTemp->valuestring = strdup(buffer);
			if(NULL != pTemp->valuestring)
			{
				return true;
			}
		});
	}

	return false;
}

bool Lib_Json::putValueNumber(cJSON *root, const char *name, unsigned int value)
{
	return GetValueNumberToObject(root, name, value);
}

bool Lib_Json::putValueNumber(cJSON *root, const char *name, int value)
{
	return GetValueNumberToObject(root, name, value);
}

bool Lib_Json::putValueNumber(cJSON *root, const char *name, unsigned long long int value)
{
	return GetValueNumberToObject(root, name, value);
}

bool Lib_Json::putValueNumber(cJSON *root, const char *name, long long int value)
{
	return GetValueNumberToObject(root, name, value);
}

bool Lib_Json::putValueNumber(cJSON *root, const char *name, double value)
{
	return GetValueNumberToObject(root, name, value);
}

bool Lib_Json::getValueString(cJSON *root, const char *name, char *buffer, unsigned int size)
{
	if(NULL != root && NULL != name && 0 < strlen(name) && NULL != buffer && 0 < size)
	{
		return GetObjectItemValueS(cJSON_String, root, pTemp, name, strncpy(buffer, pTemp->valuestring, size));
	}

	return false;
}

bool Lib_Json::getValueBoolean(cJSON *root, const char *name)
{
	cJSON *pTemp = NULL;

	if(NULL != root && NULL != name && 0 < strlen(name))
	{
		pTemp = cJSON_GetObjectItem(root, name);

		if(NULL != pTemp)
		{
			if(cJSON_True == pTemp->type)
			{
				return true;
			}
		}
	}

	return false;
}

bool Lib_Json::getValueNumber(cJSON *root, const char *name, short *value)
{
	return GetValueNumberFromObject(root, name, value, valueint);
}

bool Lib_Json::getValueNumber(cJSON *root, const char *name, unsigned short *value)
{
	return GetValueNumberFromObject(root, name, value, valueint);
}

bool Lib_Json::getValueNumber(cJSON *root, const char *name, int *value)
{
	return GetValueNumberFromObject(root, name, value, valueint);
}

bool Lib_Json::getValueNumber(cJSON *root, const char *name, unsigned int *value)
{
	return GetValueNumberFromObject(root, name, value, valuedouble);
}

bool Lib_Json::getValueNumber(cJSON *root, const char *name, long long int *value)
{
	return GetValueNumberFromObject(root, name, value, valuedouble);
}

bool Lib_Json::getValueNumber(cJSON *root, const char *name, unsigned long long int *value)
{
	return GetValueNumberFromObject(root, name, value, valuedouble);
}

bool Lib_Json::getValueNumber(cJSON *root, const char *name, double *value)
{
	return GetValueNumberFromObject(root, name, value, valuedouble);
}

bool Lib_Json::getValueArray(cJSON *root, const char *name, cJSON **value)
{
	if(NULL != root && NULL != name && 0 < strlen(name) && NULL != value)
	{
		return GetObjectItemValueS(cJSON_Array, root, pTemp, name, *value = pTemp);
	}

	return false;
}

bool Lib_Json::getValueObject(cJSON *root, const char *name, cJSON **value, bool dup)
{
	if(NULL != root && NULL != name && 0 < strlen(name) && NULL != value)
	{
		return GetObjectItemValueS(cJSON_Object, root, pTemp, name, *value = (true == dup ? cJSON_Duplicate(pTemp, 1) : pTemp));
	}

	return false;
}

bool Lib_Json::getValueNumberFromArray(cJSON *root, int *value, unsigned int n)
{
	return GetValueNumberFromArray(root, value, n, valueint);
}

bool Lib_Json::getValueNumberFromArray(cJSON *root, unsigned int *value, unsigned int n)
{
	return GetValueNumberFromArray(root, value, n, valuedouble);
}

bool Lib_Json::getValueNumberFromArray(cJSON *root, long long int *value, unsigned int n)
{
	return GetValueNumberFromArray(root, value, n, valuedouble);
}

bool Lib_Json::getValueNumberFromArray(cJSON *root, unsigned long long int *value, unsigned int n)
{
	return GetValueNumberFromArray(root, value, n, valuedouble);
}

bool Lib_Json::getValueNumberFromArray(cJSON *root, double *value, unsigned int n)
{
	return GetValueNumberFromArray(root, value, n, valuedouble);
}

bool Lib_Json::getValueStringFromArray(cJSON *root, char **value, unsigned int size, unsigned int n)
{
	bool ret = false;

	cJSON *child = NULL;

	if(NULL != root && NULL != value && 0 < size && 0 < n)
	{
		child = root->child;

		for(unsigned int i = 0; n > i; i++)
		{
			if(NULL == child || cJSON_String != child->type)
			{
				goto out;
			}

			strncpy(value[i], child->valuestring, size);

			child = child->next;
		}

		ret = true;
	}

out:
	return ret;
}

void Lib_Json::dumpAll(cJSON *root, bool useFormat)
{
	char *strJson = NULL;

	if(NULL != root)
	{
		strJson = true == useFormat ? cJSON_Print(root) : cJSON_PrintUnformatted(root);

		if(NULL != strJson)
		{
			printf("%s\n", strJson);

			free(strJson);
			strJson = NULL;
		}
	}
}

int Lib_Json::getLenght(cJSON *root)
{
	int len = 0;

	std::queue<cJSON *> q;

	cJSON *pTemp = NULL;

	if(NULL != root)
	{
		// TODO: 首先把根压入队列
		q.push(root);

		while(true != q.empty())
		{
			// TODO: 取出队列头元素
			pTemp = q.front();

			q.pop();

			// TODO: 判断元素是否有效
			if(NULL != pTemp)
			{
				// TODO: 对当前元素求个数
				len += cJSON_GetArraySize(pTemp);

//				Lib_Json::dumpAll(pTemp);

				// TODO: 指向当前元素的子链表
				pTemp = pTemp->child;

				// TODO: 循环将子链表中所有对象和数组压入队列
				while(NULL != pTemp)
				{
					if(cJSON_Object == pTemp->type || cJSON_Array == pTemp->type)
					{
						q.push(pTemp);
					}

					pTemp = pTemp->next;
				}
			}
		}
	}

	return len;
}

cJSON *Lib_Json::loadFile(const char *path)
{
	struct stat file_stat;

	FILE *fp = NULL;

	char *buffer = NULL;

	size_t readcnt = 0;

	cJSON *root = NULL;

	if(NULL == path)
	{
		goto out;
	}

	if(0 != stat(path, &file_stat))
	{
		goto out;
	}

	if(1048576 < file_stat.st_size)
	{
		goto out;
	}

	fp = fopen(path, "r");

	if(NULL == fp)
	{
		goto out;
	}

	buffer = (char *)calloc(1, file_stat.st_size + 10);

	if(NULL == buffer)
	{
		goto out1;
	}

	readcnt = fread(buffer, file_stat.st_size, 1, fp);

	if(1 != readcnt)
	{
		goto out2;
	}

	root = cJSON_Parse(buffer);

out2:
	if(NULL != buffer)
	{
		free(buffer);
		buffer = NULL;
	}
out1:
	if(NULL != fp)
	{
		fclose(fp);
		fp = NULL;
	}
out:
	return root;
}

bool Lib_Json::saveFile(cJSON *root, const char *path, bool useFormat)
{
	bool ret = false;

	FILE *pf = NULL;

	char *strJson = NULL;

	if(NULL == root || NULL == path)
	{
		goto out;
	}

	pf = fopen(path, "w+");

	if(NULL == pf)
	{
		goto out;
	}

	strJson = true == useFormat ? cJSON_Print(root) : cJSON_PrintUnformatted(root);

	if(NULL == strJson)
	{
		goto out1;
	}

	fprintf(pf, "%s\n", strJson);

	ret = true;

	goto out2;

out2:
	if(NULL != strJson)
	{
		free(strJson);
		strJson = NULL;
	}
out1:
	if(NULL != pf)
	{
		fclose(pf);
		pf = NULL;
	}
out:
	return ret;
}

void Lib_Json::freeAll(cJSON **root)
{
	if(NULL != root && NULL != *root)
	{
		cJSON_Delete(*root);
		*root = NULL;
	}
}

Lib_Json *Lib_Json::getObject(void *root)
{
	return Lib_Json::getObject((cJSON *)root);
}

Lib_Json *Lib_Json::getObject(cJSON *root)
{
	Lib_Json *obj = NULL;

	if(NULL == root)
	{
		goto out;
	}

	obj = new Lib_Json();

	if(NULL == obj)
	{
		goto out;
	}

	obj->root = cJSON_Duplicate(root, 1);

	if(NULL == obj->root)
	{
		goto out1;
	}

	goto out;

out1:
	if(NULL != obj)
	{
		delete obj;
		obj = NULL;
	}
out:
	return obj;
}

Lib_Json *Lib_Json::getObject(const char *buffer, int size)
{
	Lib_Json *obj = NULL;

	if(NULL == buffer || 0 >= size)
	{
		goto out;
	}

	obj = new Lib_Json();

	if(NULL == obj)
	{
		goto out;
	}

	obj->root = cJSON_Parse(buffer);

	if(NULL == obj->root)
	{
		goto out1;
	}

	goto out;

out1:
	if(NULL != obj)
	{
		delete obj;
		obj = NULL;
	}
out:
	return obj;
}

Lib_Json *Lib_Json::getObject(const char *path)
{
	Lib_Json *obj = NULL;

	if(NULL == path)
	{
		goto out;
	}

	obj = new Lib_Json();

	if(NULL == obj)
	{
		goto out;
	}

	obj->root = Lib_Json::loadFile(path);

	if(NULL == obj->root)
	{
		goto out1;
	}

	goto out;

out1:
	if(NULL != obj)
	{
		delete obj;
		obj = NULL;
	}
out:
	return obj;
}
#endif
