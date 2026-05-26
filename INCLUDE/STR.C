/*
   Ищем в строке указанные символы.
*/
int str_find(int i,int debug,int type,char *buffer,int def_data[100][2],int val_int[100],char val_str[100][255])
{
 int m,n,j; debug=debug;
 char str1[255],str2[255];
 if ((i=skip(i,buffer,"(")) == -1)
  return -1;
/*
   Считаем переменную под строку
*/
 if (buffer[i] == '\'')
 {
  if(text_to_str(i,buffer,str1) == -10) return -1;
  i+=strlen(str1)+2;
 }
 else
 {
  if ((m=find_var(i,buffer)) == -1)
   return -1;
  if (get_type(m) != 2)
  {
   er_in_str();
   printf("Переменная '%s' должна быть строкой или указателем на строку!",var_name[m]);
   return -1;
  }
  if (var_data[m][0] == 2)
   strcpy(str1,(char*)val_str[var_data[m][1]]);
  else
   strcpy(str1,(char*)(stUser.name+def_data[var_data[m][1]][0]));
  i+=strlen(var_name[m]);
 }
 if ((i=skip(i,buffer,",")) == -1)
  return -1;
/*
   Считаем переменную под маркер
*/
 if (buffer[i] == '\'')
 {
  if(text_to_str(i,buffer,str2) == -10) return -1;
  i+=strlen(str2)+2;
 }
 else
 {
  if ((n=find_var(i,buffer)) == -1)
   return -1;
  if (get_type(n) != 2)
  {
   er_in_str();
   printf("Переменная '%s' должна быть строкой или указателем на строку!",var_name[n]);
   return -1;
  }
  if (var_data[n][0] == 2)
   strcpy(str2,(char*)val_str[var_data[n][1]]);
  else
   strcpy(str2,(char*)(stUser.name+def_data[var_data[n][1]][0]));
  i+=strlen(var_name[n]);
 }
 if ((i=skip(i,buffer,",")) == -1)
  return -1;
/*
   Считаем переменную под флаг.
*/
 if ((j=find_var(i,buffer)) == -1)
  return -1;
 if (var_data[j][0] != 0)
 {
  er_in_str();
  printf("Переменная '%s' должна быть типа integer!",var_name[j]);
  return -1;
 }
 i+=strlen(var_name[j]);
 if ((i=skip(i,buffer,")")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;

 if (type == 1)
// { 
   strlwr(str1); strlwr(str2); 
//  printf("!%s,%s!",str1,str2);
// }
/*
   Вычисления
*/
 int a,b,c,d;
 for (a=0; a<strlen(str1); a++)
 {
//  printf("!");
  for (b=0,c=0,d=a; b<strlen(str2); b++,d++)
  {
   if (str2[b] != '?')
   {
//    printf("%c",str1[d]);
    if (str1[d] != str2[b])
    { c=0; break; }
    else
     c=1; //printf("%c",str1[d]); }
   }
  }
  if (c == 1)
   break;
 }
 val_int[var_data[j][1]]=c;
 return i;
}
/*
   Считаем длину строки (до первого символа 0) "buffer" с символа i.

   str_len [ строка (str или define), переменная под размер (int или real) ]

   При
*/
int str_len(int i,int debug,char *buffer,int def_data[100][2],int val_int[100],char val_str[100][255],long val_real[100])
{
 int m,n,tempi; long templ; debug=debug;
 if ((i=skip(i,buffer,"(")) == -1)
  return -1;
/*
   Считаем переменную под строку
*/
 if ((m=find_var(i,buffer)) == -1)
  return -1;
//
 if (get_type(m) != 2)
 {
  er_in_str();
  printf("Переменная '%s' должна быть строкой или указателем на строку!",var_name[m]);
  return -1;
 }
 i+=strlen(var_name[m]);
 if ((i=skip(i,buffer,",")) == -1)
  return -1;
/*
   Считаем переменную под размер.
*/
 if ((n=find_var(i,buffer)) == -1)
  return -1;
 if ((var_data[n][0] > 1) || ((var_data[n][0] == 3) && (def_data[var_data[n][0]][1] > 1)))
 {
  er_in_str();
  printf("Переменная '%s' должна быть числом (integer, real) или указателем на число!",var_name[n]);
  return -1;
 }
/*
   Считаем размер строки.
*/
 void *p = (int*)(stUser.name+def_data[var_data[m][1]][0]);
 if (var_data[m][0] == 2)
 {
  tempi=strlen(val_str[var_data[m][1]]);
  templ=strlen(val_str[var_data[m][1]]);
 }
 if ((var_data[m][0] == 3) && (get_type(m) == 2))
 {
  tempi=strlen((char *)p);
  templ=strlen((char *)p);
 }
/*
   Посчитанный размер положим в переменную.
*/
 if ((var_data[n][0] == 0) || (var_data[n][0] == 1))
 {
  if (var_data[n][0] == 0)
   val_int[var_data[n][1]] = tempi;
  else
   val_real[var_data[n][1]] = templ;
 }
 if (var_data[n][0] == 3)
 {
  if (def_data[var_data[n][1]][1] == 0)
   *(int *)p = tempi;
  if (def_data[var_data[n][1]][1] == 1)
   *(long *)p = templ;
 }
 i+=strlen(var_name[n]);
 if ((i=skip(i,buffer,")")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;
 return i;
}