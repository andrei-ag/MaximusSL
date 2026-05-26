#include <dos.h>
/* */
char calc(int i,char *buffer,char *result,char *end);
int count_var(int i,char *buffer,char *end,char op_data[50],char vr_val[100][50]);
/* ════════════════════════════════════════════════════════════════════════════
   proc_set - функция установки нового значения переменной

   on start      i - байт с которого сканировать буффер
		 buffer - буффер который нужно сканировать
		 end[0] - символ конца строки установки
   on exit       0 - действие проведено успешно
*/
int proc_set(int i,char *string,char *end)
{
 int m,n,j,k,x;
 char temp_s[255];
 char result[255];
 char buffer[255];

 strcpy(buffer,string);

 i=skip_space(i,buffer);
 if ((m=find_var(i,buffer)) == -1)
  return -1;
 i+=strlen(var_name[m]);

 i=skip_space(i,buffer);
 if (buffer[i] != '=')
  return 0;
 else
  i++;
 i=skip_space(i,buffer);
 if (debug == 1)
  fprintf(debug_file,"Переменной %s установленно значение ",var_name[m]);
//
 if (get_type(m) == 2)
 {
  if (buffer[i] == '\'')
  {
   if (text_to_str(i,buffer,result) == -10) return -1;
   i+=str_size(i,buffer,"'");
  }
  else
  {
   if ((n=find_var(i,buffer)) == -1)
    return -1;
   if (var_data[n][0] == 2)
    strcpy(result,(char*)val_str[var_data[n][1]]);
   else
   {
    void *p=(int*)(stUser.name+def_data[var_data[n][1]][0]);
    strcpy(result,(char *)p);
   }
   i+=strlen(var_name[n]);
  }
//
  void *p=(int*)(stUser.name+def_data[var_data[m][1]][0]);
  if (var_data[m][0] == 2)
   strcpy((char*)val_str[var_data[m][1]],result);
  if ((var_data[m][0] == 3) && (get_type(m) == 2))
   strcpy((char*)p,result);
 }
 else
 {
  for (; ; )
  {
   j = str_size(i,buffer,end);
   if ((j == 0) || (j < str_size(i,buffer,")")) || (end[0] == ')'))
   {
    if (calc(i,buffer,result,end) == -1)
     return -1;
    break;
   }
   j = str_size(i,buffer,")");
//   printf("j%i i%i",j,i);
   k=j+i+1; x=str_size(k,buffer,end)+1;
//   printf("!%i k%i",x,k);

   for (j+=i; buffer[j-1] != '('; )
    j--;
   if (calc(j,buffer,result,")") == -1)
    return -1;
   buffer[j-1] = 0;

   for (j=0; j<x; j++,k++)
    temp_s[j] = buffer[k];
   temp_s[j] = 0;

   strcat(buffer,result);
   strcat(buffer,temp_s);
//   printf("\n",buffer);
  }

  if ((var_data[m][0] == 0) || (var_data[m][0] == 1))
  {
   if (get_type(m) == 0)
    val_int[var_data[m][1]] = atoi(result);
   if (get_type(m) == 1)
    val_real[var_data[m][1]] = atol(result);
  }

  if (var_data[m][0] == 3)
  {
   if (get_type(m) == 0)
    def_data[var_data[m][1]][0] = atoi(result);
   if (get_type(m) == 1)
    def_data[var_data[m][1]][0] = atol(result);
  }
 }
 return 0;
}
/* ════════════════════════════════════════════════════════════════════════════
*/
char calc(int i,char *buffer,char *result,char *end)
{
 int m,n,k;
 long res,num;
 char op_data[50];
 char vr_val[100][50];

 if ((m=count_var(i,buffer,end,op_data,vr_val)) == -1)
  return -1;
// printf("%i ",m);

 if (m == 0)
 {
  res=atol((char *)vr_val[0]);
  ltoa(res,result,10);
  return (char)result;
 }
//
 for (n=0; n<=m; )
 {
//  if (m == 1)
//   break;
  if (op_data[n] == '*' || op_data[n] == '/')
  {
   res=atol((char *)vr_val[n]);
   if (op_data[n] == '*')
    res*=atol(vr_val[n+1]);
   if (op_data[n] == '/')
    res/=atol(vr_val[n+1]);

   ltoa(res,vr_val[n],10);
   m--;
//   printf("%i ",m); delay(1500);
   if (m <= 1)
   {
//    printf("@");
    strcpy((char*)vr_val[n+1],(char*)vr_val[n+2]);
    op_data[n] = op_data[n-1];
    break;
   }
   else
   {
    for (k=n+1; k-1!=m; k++)
    {
     strcpy((char*)vr_val[k],(char*)vr_val[k+1]);
     op_data[k-1] = op_data[k];
////     printf("!%s%c",vr_val[k],op_data[k]);
    }
   }
  }
  else
   n++;
 }/*
   int j;
   printf ("!");
   for (j=0; j<=m; j++)
    printf("%s%c",vr_val[j],op_data[j]);
   printf("!");
   getch();*/
 for (n=0; m!=0; )
 {
  res=atol((char *)vr_val[n]);
  if (op_data[n] == '+')
   res+=atol(vr_val[n+1]);
  if (op_data[n] == '-')
   res-=atol(vr_val[n+1]);
//  printf("|%s%c%s|",vr_val[n],op_data[n],vr_val[n+1]);

  ltoa(res,vr_val[n],10);
  m--;
  for (k=n+1; k-1!=m; k++)
  {

   strcpy((char*)vr_val[k],(char*)vr_val[k+1]);
   op_data[k-1] = op_data[k];
  }
 }
//
 strcpy(result,(char*)vr_val[0]);
// printf("=%s",vr_val[0]);
 return (char)result;
}
/* ════════════════════════════════════════════════════════════════════════════
*/
int count_var(int i,char *buffer,char *end,char op_data[50],char vr_val[100][50])
{
 int m,n,type;

// printf("!%c%c'%c'",buffer[i],buffer[i+1],end[0]);
 for (m=0; ; m++)
 {
  i=skip_space(i,buffer);
  if ((buffer[i] == '-') || ((buffer[i] >= '0') && (buffer[i] <= '9')))
  {
   num_to_str(i,buffer,vr_val[m]);
   i+=strlen(vr_val[m]);
  }
  else
  {
   n=find_var(i,buffer);
   if (n == -1)
    return -1;
   type=get_type(n);
   if (var_data[n][0] < 2)
   {
    if (type == 0)
     itoa(val_int[var_data[n][1]],vr_val[m],10);
    else
     ltoa(val_real[var_data[n][1]],vr_val[m],10);
   }
   if (var_data[n][0] == 3)
   {
    if (type == 0)
     itoa(def_data[var_data[n][1]][0],vr_val[m],10);
    else
     ltoa(def_data[var_data[n][1]][0],vr_val[m],10);
   }
   i+=strlen(var_name[n]);
  }
  i=skip_space(i,buffer);
  if (buffer[i] == end[0])
//  {
//   printf("!c%i i%i!",str_cur,i);
   break;
//  }
  if ((buffer[i] != '-') && (buffer[i] != '+') && (buffer[i] != '*') && (buffer[i] != '/'))
  {
   er_in_str();
   printf("Незвестное действие - '%c', в позиции %i!",buffer[i],i);
   return -1;
  }
  op_data[m] = buffer[i]; i++;
 }
 return m;
}