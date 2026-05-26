int var_name_size(int i,int max_var_name_size,char *buffer)
{
 int j;
 for (j=0; j<=max_var_name_size; i++,j++)
 {
  if (buffer[i] != '_')
  {
   if ((buffer[i] < '0') || (buffer[i] > 'z'))
    return j;
   if ((buffer[i] > '9') && (buffer[i] < 'A'))
    return j;
   if ((buffer[i] > 'Z') && (buffer[i] < 'a'))
    return j;
  }
 }
 return j;
}
/* 
   skip_space
*/
int skip_space(int i,char *buffer)
{
 for (; i<255; i++)
  if (buffer[i] != ' ')
   return i;
 return -1;
}
/*
   str_size - размер строки, строка заканчивается символом ";"

   on start      buffer - строка, размер которой нужно вычислить
   on exit       0 - ошибка иначе возвращается размер строки (0-255 символов)

*/
int str_size(int i, char *buffer, char *end)
{
 int j;
 for (j=0; i<=255; i++,j++)
  if (buffer[i] == end[0])
   return j;
 return 0;
}
/*
   file_size
*/
long filesize(FILE *stream)
{
 long curpos, length;

 curpos = ftell(stream);
 fseek(stream, 0L, SEEK_END);
 length = ftell(stream);
 fseek(stream, curpos, SEEK_SET);
 return length;
}
/*
   if_param_end - подошли к концу строки параметров команды?

   on start      i - байт с которого сканировать строку
                 buffer - строка которую нужно сканировать
   on exit       -1 - подошли, но команда закончена неверно
                 0 - подошли к концу
                 1 - не подошли
*/
int if_param_end(int i, char *buffer)
{
 if (buffer[i] == ')')
  if (buffer[i+1] == ';')
   return 0;
  else
   return -1;
 return 1;
}
/*
*/
void display_buf(int i,int n,char *buffer)
{
 n += i;
 for (; i<n; i++)
  printf("%c",buffer[i]);
}

void display_var_type(int i)
{
 if (i == 0)
  printf("integer");
 else
 {
  if (i == 1)
   printf("real");
  else
   printf("string");
 }
}

int skip_first(int i,char *buffer)
{
 i=skip_space(i,buffer);
 if (buffer[i] == '(')
  i++;
 return skip_space(i,buffer);
}
