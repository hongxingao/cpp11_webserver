#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
Ini�����ļ��������£�
�Էֺ�';'��ͷΪע��
���������ڵ�Ϊ���ڡ� [section]
һ���ڿɰ��������ֵ�� ��key=value��
*/
int IniReadStr(const char* fname, const char* section, const char* key, char* value, int size);
long GetRowSize(FILE* fp);
bool GetRow(FILE* fp, char* row, long count, long fpBegin);//��ȡһ��
bool IsTrueSection(char* line, const char* section, int length, int section_len);//�����Ƿ��ǶԵĽ�
bool IsSection(char* line, int length);
bool GetTrueKeyValue(char* row, int length, const char* key, const int key_len, char* value, int* size);
char* LeftTrim(char* begin, char* end);
char* RightTrim(char* end, char* begin);
static char* Trim(char* str, int* length);

/// <summary>
/// ��ȡ�����ļ�
/// </summary>
/// <param name="fname">In:�ļ�·��</param>
/// <param name="section">In:����</param>
/// <param name="key">In:����</param>
/// <param name="value">In,Out:ֵ�Ļ�����</param>
/// <param name="size">In:Ҫ��ȡֵ�ĳߴ�</param>
/// <returns></returns>
int IniReadStr(const char* fname, const char* section, const char* key, char* value, int size)
{
    int result = -2;
    if (fname == NULL)
        result = -1;
    else if (section == NULL)
        result = -2;
    else if (key == NULL)
        result = -3;
    else if (value == NULL || size <= 0)
        result = -4;
    else
    {
        FILE* fp = fopen(fname, "r");
        if (fp != NULL)
        {
            int sec_len = strlen(section);
            int key_len = strlen(key);
            long count;
            long fp_begin, fp_next;
            bool is_found_sec = false;
            char* begin_ptr;
            char* end_ptr;
            int line_size = 256;
            char* row = (char*)malloc(line_size);
            
            if (row != NULL)
            {
                do
                {
                    fp_begin = ftell(fp);
                    count = GetRowSize(fp);
                    if (line_size < count)
                    {
                        line_size = count;
                        free(row);
                        row = (char*)malloc(line_size + 1);
                        if (row == NULL)
                            break;
                    }
                    if (count < 2)
                        ;
                    else if (GetRow(fp, row, count, fp_begin))
                    {
                        end_ptr = row + count;
                        begin_ptr = LeftTrim(row, end_ptr);
                        if (*begin_ptr == ';')
                            ;
                        else if (!is_found_sec )
                        {
                            if (*begin_ptr == '[' && end_ptr - begin_ptr >= sec_len + 2
                                && IsTrueSection(begin_ptr + 1, section, end_ptr - begin_ptr - 1, sec_len))
                                is_found_sec = true;
                        }
                        else if (end_ptr - begin_ptr >= 2)
                        {
                            if (*begin_ptr == '[' && IsSection(begin_ptr + 1, end_ptr - begin_ptr - 1))//û�ҵ����������ҵ�����һ����
                            {
                                result = -3;
                                break;
                            }
                            else if (end_ptr - begin_ptr > key_len && GetTrueKeyValue(begin_ptr, end_ptr - begin_ptr, key, key_len, value, &size))
                            {
                                result = size;
                                break;
                            }
                        }
                    }
                    else
                        break;
                                      
                } while (!feof(fp));
                if (row != NULL)
                    free(row);
            }
            else
                printf("Error malloc failed");

            fclose(fp);
        }
        else
            result = -1;
        
    }
    return result;
}
    
long GetRowSize(FILE* fp)
{
    int count = 0;
    int ch = fgetc(fp);
    while (ch != EOF && ch != '\n')
    {
        ++count;
        ch = fgetc(fp);
    }

    return count;
}

bool GetRow(FILE* fp, char* row, long count, long fpBegin)
{
    bool result = false;
    long next = ftell(fp);
    fseek(fp, fpBegin - next, SEEK_CUR);
    if (fread(row, 1, count, fp) == count)
    {
        long cur = ftell(fp);
        fseek(fp, next - ftell(fp), SEEK_CUR);   
        //fgetc(fp);
        //row[*count] = '\0';   
        result = true;
    }

    return result;
}

bool IsSection(char* line, int length)
{
    char *end_ptr = RightTrim(line + length, line);

    return *end_ptr == ']'; 
}

bool IsTrueSection(char* line, const char* section, int length, int section_len)
{
    bool result = false;
    char* end_ptr;
    char* begin_ptr;
    end_ptr = RightTrim(line + length, line);
    if (*end_ptr == ']' && end_ptr - line >=  section_len)
    {
        end_ptr = RightTrim(end_ptr, line) + 1;
        if (end_ptr - line >= section_len)
        {
            begin_ptr = LeftTrim(line, end_ptr);
            if (end_ptr - begin_ptr == section_len && memcmp(begin_ptr, section, section_len) == 0)
                result = true;
        }
    }

    return result;
}

bool GetTrueKeyValue(char* row, int length, const char* key, const int key_len, char* value, int* size)
{
    bool result = false;
    if (memcmp(row, key, key_len) == 0)
    {
        char* begin_ptr = LeftTrim(row + key_len, row + length);
        if (*begin_ptr == '=')
        {
            char* end_ptr = RightTrim(row + length, begin_ptr);
            if (end_ptr - begin_ptr > 0)
            {
                char* val_begin = LeftTrim(begin_ptr + 1, end_ptr);
                if (end_ptr >= val_begin)
                {
                    if (end_ptr - val_begin + 1 < * size)
                        *size = end_ptr - val_begin + 1;
                    memcpy(value, val_begin, *size);
                    result = true;
                }
            }     
        }
    }

    return result;
}

char* LeftTrim(char* begin, char* end) 
{
    while (begin < end && (unsigned char)*begin <= ' ')
        begin++;

    return begin;
}

char* RightTrim(char* end, char* begin) 
{   
    end--;
    while (end >= begin && (unsigned char)*end <= ' ')
        end--;
        
    return end;
}

static char* Trim(char* str, int* length)
{
    char* end = str + *length;
    str = LeftTrim(str, end);
    end = RightTrim(end, str);
    *length = end - str + 1;

    return str;
}