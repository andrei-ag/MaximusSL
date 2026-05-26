#define no    1
#define yes   0
/* ────────────────────────────────────────────────────────────────────────────
   get_if - получим результат сравнения двух переменных

   on start  m - номер первой переменной
	     n - номер второй переменной
	     cmp - знак сравнения
   on exit   выполняется условие
*/
int get_if(int cmp,int type,int num_int[2],long num_real[2])
{
 if (type == 1) // real
  switch (cmp)
  {
   case 1: if (num_real[0] >= num_real[1]) return yes; else return no;
   case 2: if (num_real[0] > num_real[1])  return yes; else return no;
   case 3: if (num_real[0] <= num_real[1]) return yes; else return no;
   case 4: if (num_real[0] < num_real[1])  return yes; else return no;
   case 5: if (num_real[0] != num_real[1]) return yes; else return no;
   case 6: if (num_real[0] == num_real[1]) return yes; else return no;
  }
 else            // integer
  switch (cmp)
  {
   case 1: if (num_int[0] >= num_int[1]) return yes; else return no;
   case 2: if (num_int[0] > num_int[1])  return yes; else return no;
   case 3: if (num_int[0] <= num_int[1]) return yes; else return no;
   case 4: if (num_int[0] < num_int[1])  return yes; else return no;
   case 5: if (num_int[0] != num_int[1]) return yes; else return no;
   case 6: if (num_int[0] == num_int[1]) return yes; else return no;
  }
 return -1;
}
/* ────────────────────────────────────────────────────────────────────────────
    if_yes_no - возвращает результат сравнения переменных

    on start    i - указатель на начало строки (имя переменной, знак ...)
		buffer - строка
    on end      0 - условие выполнилось, 1 - условие не выполнилось
	       -1 ошибка
*/
int if_yes_no(int i,int if_res[2],char *buffer)
{
 int m,n,j,cmp,type;
 char temp_str[255];
/*
   Массивы для хранения двух сравниваемых значений
*/
 int num_int[2];
 long num_real[2];
 char num_str[2][255];
 i=skip_space(i,buffer);
 if (buffer[i] == '#')
 {
  if_res[0]=i; if_res[1]=1;
  return (int)if_res;
 }
/*
   Если первый символ в указателе i - "'", то значит это текст строки.
   Перенесем текст в массив
*/
 if (check_sk(i,buffer) == -1) return -1;
 if (buffer[i] == '\'')
 {
  if(text_to_str(i,buffer,num_str[0]) == -10) return -1;
  i+=strlen(num_str[0])+2;    // пропустим текст и маркеры начала и конца "'".
  type = 2;                   // тип переменной - строка
 }
/*
   Указатель i указывает на первую переменную, нам неизвестно, число это или
   имя переменной. Имя переменной не может начинаться с минуса или числа. Если
   первый символ минус или число от 0 до 9, то это число.
*/
 else
 {
  if ( (buffer[i] == '-') || ((buffer[i] >= '0') && (buffer[i] <= '9')) )
  {
   num_to_str(i,buffer,temp_str);
   num_int[0] = atoi(temp_str); // переведем число в тип integer
   num_real[0] = atol(temp_str); // переведем число в тип real
   i += strlen(temp_str);
   type = 4; // значит первая переменная число и может быть любым типом (real, int)
  }
  else
  {
/*
   Работаем с переменной, считаем размер ее имяни, ищем в памяти. Определяем
   тип, сохраняем значение в массиве.
*/
   m=find_var(i,buffer);
   if (m == -1)
    return -1;
   i += strlen(var_name[m]); // перенесем указатель на знак
   type = var_data[m][0];    // тип найденной переменной
   if (type == 0)            // если тип - integer
    num_int[0] = val_int[var_data[m][1]];
   if (type == 1)
    num_real[0] = val_real[var_data[m][1]];
   if (type == 2)
    strcpy(num_str[0],val_str[var_data[m][1]]);
   if (type == 3)
   {
    type=def_data[var_data[m][1]][1];
    void *p = (int*)(stUser.name+def_data[var_data[m][1]][0]);
    if (type == 0)
     num_int[0] = *(int *)p;
    if (type == 1)
     num_real[0] = *(long *)p;
    if (type == 2)
     strcpy(num_str[0],(char*)p);
   }
  }
 }
/*
   Теперь определим знак сравнения.
*/
 i=skip_space(i,buffer);
 if ((buffer[i] == '>') && (buffer[i+1] == '='))
  cmp = 1;
 else
  if (buffer[i] == '>')
   cmp = 2;

 if ((buffer[i] == '<') && (buffer[i+1] == '='))
  cmp = 3;
 else
  if (buffer[i] == '<')
   cmp = 4;

 if ((buffer[i] == '!') && (buffer[i+1] == '='))
  cmp = 5;
 else
  if ((buffer[i] == '=') && (buffer[i+1] != '='))
   cmp = 6;

 if ((cmp < 1) || (cmp > 6))
 {
  er_in_str();
  printf ("Неизвестный знак сравнения '%c%c'!",buffer[i],buffer[i+1]);
  return -1;
 }

 if ((cmp == 1) || (cmp == 3) || (cmp == 5)) i++;
 i++;
/*
   Определим тип второй переменной.
*/
 i=skip_space(i,buffer);
 if (check_sk(i,buffer) == -1) return -1;
 if (buffer[i] == '\'')
 {
  if (type != 2)
  {
   er_in_str();
   printf("Нельзя сравнивать переменные разных типов!");
   return -1;
  }
  if (text_to_str(i,buffer,num_str[1]) == -10) return -1;
  i+=strlen(num_str[1])+2;    // пропустим текст и маркеры начала и конца "'".
  type = 2;                   // тип переменной - строка
 }
//
 else
 {
  if ( (buffer[i] == '-') || ((buffer[i] >= '0') && (buffer[i] <= '9')) )
  {
   num_to_str(i,buffer,temp_str);
   num_int[1] = atoi(temp_str); // переведем число в тип integer
   num_real[1] = atol(temp_str); // переведем число в тип real
   i += strlen(temp_str);
   if (type == 4)
    type = 1;     // если сравниваются два числа, то их тип real;
  }
  else
  {
   i=skip_space(i,buffer);
   n=find_var(i,buffer);
   if (n == -1)
    return -1;
   i += strlen(var_name[n]);
   if (type == 4)
    if (var_data[n][0] == 3)
     type = def_data[var_data[n][1]][1];
    else
     type = var_data[n][0];

   if (((var_data[n][0] != 3) && (type != var_data[n][0])) || ((var_data[n][0] == 3) && (type != def_data[var_data[n][1]][1])))
   {
    er_in_str();
    printf("Нельзя сравнивать переменные разных типов!");
    return -1;
   }
   if ((var_data[n][0] >= 0) || (var_data[n][0] <= 2))
   {
    if (type == 0)            // если тип - integer
     num_int[1] = val_int[var_data[n][1]];
    if (type == 1)
     num_real[1] = val_real[var_data[n][1]];
    if (type == 2)
     strcpy(num_str[1],val_str[var_data[n][1]]);
   }
   if (var_data[n][0] == 3)
   {
    type=def_data[var_data[n][1]][1];
    void *p = (int*)(stUser.name+def_data[var_data[n][1]][0]);
    if (def_data[var_data[n][1]][1] == 0)
     num_int[1] = *(int *)p;
    if (def_data[var_data[n][1]][1] == 1)
     num_real[1] = *(long *)p;
    if (def_data[var_data[n][1]][1] == 2)
     strcpy(num_str[0],(char*)p);
   }
  }
 }
 if_res[0]=i;
 if (type == 2) // строка
 {
  j=strcmp(num_str[0],num_str[1]);
  if (cmp == 5)
   if (j == 0)
   {
    if_res[1]=1;
    return (int)if_res;
   }
   else
   {
    if_res[1]=0;
    return (int)if_res;
   }

  if (cmp == 6)
   if (j == 0)
   {
    if_res[1]=0;
    return (int)if_res;
   }
   else
   {
    if_res[1]=1;
    return (int)if_res;
   }
 }
 else
  if_res[1]=get_if(cmp,type,num_int,num_real);
 return (int)if_res;
}
/* ════════════════════════════════════════════════════════════════════════════
   proc_if - функция сравнения двух переменных одного типа

   сравниваться обязательно должны переменные!

   on start      i - байт с которого сканировать буффер
		 buffer - буффер который нужно сканировать
   on exit       0 - действие проведено успешно
*/
int proc_if(int i,int type,char *buffer,int label_max,int label_loc[100],char label_name[100][20])
{
 int m,n,k;
 int if_r[2];
 int if_all[20];
 char temp_str[255];
 strcpy(temp_str,buffer);

 if (type == 0)
 {
  if ((i=skip(i,buffer,"(")) == -1)
   return -1;
 }
 else
 {
  if (type == 1)
  {
   if (buffer[i] == '(')
    i++;
  }
 }

 for (m=1; ; )
 {
  if (if_yes_no(i,if_r,temp_str) == -1)  // условие выполнилось?
   return -1;
  i=if_r[0];
  i=skip_space(i,temp_str);

  if_all[m]=if_r[1]; m++;
  if (temp_str[i] == ')')
   break;
  if (temp_str[i] == '|')
   if_all[0] = 4;
  else
   if_all[0] = 5;
  i++;
 }
 if (m == 2)
 {
  if (if_all[1] == 0)           // условие выполнилось? перейдем к строке
  {
   if_yes:
   if (type == 0)
   {
    if ((i=skip(if_r[0],temp_str,")")) == -1)
     return -1;
    if(proc_go_to(i,temp_str,label_max,label_loc,label_name) == -1)
     return -1;
   }
   return 0;
  }
  return 0;
 }
 else
 {
  m--;
  for (k=0,n=1; n<=m; n++)
   k+=if_all[n];
  if ((if_all[0] == 4) && (k < m))
   goto if_yes;
  if ((if_all[0] == 5) && (k == 0))
   goto if_yes;
  if (type == 0)
  {
   if ((i=skip(if_r[0],temp_str,")")) == -1)
    return -1;
  }
  return 1;
 }
}