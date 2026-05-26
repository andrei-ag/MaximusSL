#include <io.h>
#include <dos.h>
#include <conio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>
#include "\user.h"
#include "\other.c"

const max_lab = 100;
const max_var = 100;                   // максимальное количество переменных одного типа
const max_var_name_size = 20;          // максимальный размер имяни переменной
const max_macros = 40;                 // количество комманд

union _dos_stamp
{
 struct { word da : 5; word mo : 4; word yr : 7; } date;
 struct { word ss : 5; word mm : 6; word hh : 5; } time;
 dword b;
} st;

_usr stUser;
FILE *scr_file,*debug_file;
int debug,param_loc,command_no,where,where_i;
/*
   для хранения имен и значений переменных
*/
int cur_def=0;
int def_data[100][2];
int val_int[100];                  // ... типа int
char val_str[100][255];            // ячейки под значения переменных типа char
long val_real[100];                // ячейки под значения переменных типа real

int var_no[3];                     // указатели на свободные номера переменных
int var_name_no;
int  var_data[400][2];             // тип переменной
char var_name[400][20];            // массив под имена переменных
/*
   переменные под комманды
*/
int str_max, str_cur;
long str_off, str_loc[5000];
char buf[255];                           // буффер под строку
char *macros[40] = { "integer ", "real ", "string ", "write", "read",
		     "if", "go to ", "for", "=", "end_for", "exit", ":",
		     "fopen", "fclose", "fread", "fwrite", "fcreate",
		     "fseek", "rd_user", "wr_user", "pause", "define",
		     "flength", "dl_user", "byte_to_num", "int_to_byte",
		     "date_to_num", "str_len", ";", "ftxt", "break", "cls",
		     "fstr", "get_date", "feof", "str_find", "stri_find",
		     "sub", "call", "ret"
		   };
// ────────────────────────────────────────────────────────────────────────────
void er_in_str();
void no_var_in_mem(int i, int j, char *buffer);
int cmp_var(int i, int m, char *buffer);
int mem_srch(int i, int j, char *buffer);
int proc_load(int command_no,int i,char *buffer);
int proc_read(int i, char *buffer);
int proc_write(int i,int type,char *buffer);
int check_macro(int i, int j, char *buf);
int scan_data(char *buffer, int length);
int process_command(int type,int label_max,int label_loc[100],char label_name[100][20],int sub_max,int sub_loc[100],char sub_name[100][20]);
char num_to_str(int i,char *string,char *temp);
char text_to_str(int i,char *string,char *temp);
int find_var(int i,char *string);
int set_many(int i,int str_cur,char *string,char *end);
int load_one_var(int command_no,int i,char *buffer);
/*
    комманды - цикл "for", выполнение подпрограмм "call"
    переход к метке - "go to" и переход к подпрограмме "call"
*/
int proc_call(int i, char *buffer,int label_max,int label_loc[100],char label_name[100][20],int sub_max,int sub_loc[100],char sub_name[100][20]);
int proc_for(int i, char *buffer,int label_max,int label_loc[100],char label_name[100][20],int sub_max,int sub_loc[100],char sub_name[100][20]);
int cmp_lab(int i, int m, char *buffer,char label_name[100][20]);
int proc_go_to(int i, char *buffer,int label_max,int label_loc[100],char label_name[100][20]);
int label_srch(int i, int j, char *buffer,int label_max,char label_name[100][20]);
//
int skip(int i,char *buffer,char *one_char);
int proc_getdate(int i,char *buffer);
//
int get_handle(int i,char *buffer);
int proc_fop_cr(int i,int op_cr,char *buffer);
int proc_fclose(int i,char *buffer);
int proc_fr_w(int i,int r_w,char *buffer);
int proc_fseek(int i,char *buffer);
int proc_filelen(int i,char *buffer);
int proc_feof(int i,char *buffer);
//
int get_type(int i);
int check_sk(int i, char *buffer);
int proc_rw_user(int i,int r_w,char *buffer);
//
#include "\proc_if.c"
#include "\proc_set.c"
#include "\str.c"
#include "\usr.c"
/* ════════════════════════════════════════════════════════════════════════════
   Главная программа.
*/
void main(int argc, char *argv[])
{
/*
   данные по меткам и подпрограммам.

   label_loc - под расположение меток (номер строки)
   label_name - имя метки (заканчивается 0)
*/
 int  label_max, sub_max;
 int  label_loc[100], sub_loc[100];
 char label_name[100][20], sub_name[100][20];
//
 int i,j,m,k;
 clrscr();
 var_name_no = 0; var_no[0] = 0; var_no[1] = 0; var_no[2] = 0; where = 0;
// argc=2;
// scanf("%s",argv[1]);
// argv[1]="bad_user.msl";
 printf("Maximus Script Language version 010 [beta]\n");
 if ((argc < 2) || (argc > 4))
 {
  printf("Неверное количество (%i) параметров командной строки!",argc-1);
  exit(1);
 }

 if (argc > 3+1)
 {
  printf("Слишком много параметров командной строки (%i, а должно быть не более 2)!",argc-1);
  exit(1);
 }
//
 if (argc == 1+1)
 {
  debug = 0;
  if ((scr_file = fopen(argv[1], "rt")) == NULL)
  {
   printf("Немогу открыть указанный (%s) скрипт файл!",argv[1]);
   exit(1);
  }
 }

 if (argc == 3+1)
 {
  debug = 1;
  if (strcmp("-d",argv[1]) != 0)
  {
   printf("Параметр '%s' не верный (должен быть '-d')!",argv[1]);
   exit(1);
  }
  if ((scr_file = fopen(argv[2], "rt")) == NULL)
  {
   printf("Немогу открыть указанный (%s) скрипт файл!",argv[2]);
   exit(1);
  }
  if ((debug_file = fopen(argv[3], "wt")) == NULL)
  {
   printf("Немогу открыть указанный (%s) файл отчета!",argv[3]);
   fclose(scr_file);
   exit(1);
  }
 }
/* */
 clrscr();
/*
    Цикл расчета кол-ва строк в скрипте, и их размер.
*/
 if (debug == 1)
  fprintf(debug_file,"Расчет количества стpок в скpипте (и их pазмеpа), а так же pазбоp меток.\n");
 for (i = 0, str_off = 0, sub_max = 0, label_max = 0; str_off<filesize(scr_file); i++)
 {
  str_loc[i] = str_off;
  fread(buf, 255, 1, scr_file);
  j = str_size(0,buf,"\x0a");            // размер строки
  if (j <= 0)
   str_off = filesize(scr_file);
  else
   str_off += j + 2;                // прочитанные байты + перевод каретки
  fseek(scr_file,str_off,SEEK_SET);
/*
   Неизвестные комманды не разбираем.
*/
  if ((k=scan_data(buf,j)) == -1)
  {
   str_cur=i;
   er_in_str();
   printf ("Неизвестная команда '");
   display_buf(0,str_size(0,buf,"\x0a"),buf);
   printf ("...'!");
   fclose(scr_file);
   exit(1);
  }
//  k=scan_data(buf,j);
/*
   Если нам попалась метка, занесем ее имя и строку на которой она обнаружена
   в базу.
*/
  if ((command_no == 11) || (command_no == 37))
  {
   if (label_max > max_lab)
   {
    er_in_str();
    printf("Максимальное количество ");
    if (command_no == 11)
     printf("меток");
    else
     printf("подпрограмм");
    printf(" в скрипте %i!",max_lab);
    goto exit_scr;
   }
//-----------------------
   if (command_no == 11)
    k=skip_space(k+1,buf);    // k - начало имяни метки
   else
    k=strlen(macros[command_no]); // смещение параметров в строке   
   k=skip_space(k,buf);
   m=str_size(k,buf,";");  // m - размер имяни метки
   if ((m > max_var_name_size) || (m <= 0))
   {
    er_in_str();
    printf("Максимальный размер имяни метки/подпрограммы %i (тут он %i)!",max_var_name_size,m);
    goto exit_scr;
   }
   if (command_no == 11)
   {
    for (j=0; j<m; j++,k++)
     label_name[label_max][j]=buf[k]; // перенесем имя метки в базу
    label_name[label_max][j]=0;       // в конце имени ставим метку - 0
    label_loc[label_max]=i;       // расположение текущей метки (номер строки)
    label_max++;
   }
   else
   {
    for (j=0; j<m; j++,k++)
     sub_name[sub_max][j]=buf[k]; // перенесем имя подпрограммы в базу
    sub_name[sub_max][j]=0;       // в конце имени ставим метку - 0
    sub_loc[sub_max]=i;       // расположение текущей подпрограммы (номер строки)
    sub_max++;
   }

   if (debug == 1)
   {
    fprintf(debug_file,"В строке %i, найдена ",i);
    if (command_no == 11)
     fprintf(debug_file,"метка '%s'\n",label_name[label_max-1]);
    else
     fprintf(debug_file,"подпрограмма '%s'\n",sub_name[sub_max-1]);
   }
  }
//
 }
  str_loc[i] = filesize(scr_file)+1;
 str_max = i;
/*
   Главный цикл - запускает команды из строк, пока не исполнит все.
   Выход происходит только если команда выполнила ошибку или встретили команду
   выхода "exit".
*/
 for (str_cur=0; str_cur<str_max; str_cur++)
 {
  k=process_command(where,label_max,label_loc,label_name,sub_max,sub_loc,sub_name);
  if ((k < 0) || (k == 10)) // выходим если произошла ошибка или встретили комманду exit
   break;
 }
 exit_scr:
 fclose(scr_file);
 if (debug == 1)
  fclose(debug_file);
}
/* ════════════════════════════════════════════════════════════════════════════
    Выполняем одну комманду находящуюся в строке номер "str_cur".

    on exit   -1 - комманда выполнилась с ошибкой, иначе все в порядке
*/
int process_command(int where,int label_max,int label_loc[100],char label_name[100][20],int sub_max,int sub_loc[100],char sub_name[100][20])
{
 int i,j; where_i=0;

 _asm mov   ah,00001
 _asm int   16h
 _asm cmp   ax,0011bh
 _asm jnz   continue_check
      return 10;
 continue_check:
 fseek(scr_file,str_loc[str_cur],SEEK_SET);
 fread(buf,255,1,scr_file);

 param_loc=scan_data(buf,(str_loc[str_cur+1]-str_loc[str_cur]-1));
 if (debug == 1)
  fprintf(debug_file,"Стpока %i: Выполнили команду '%s'\n",str_cur,macros[command_no]);
 if (param_loc < 0)
  return param_loc;

 param_loc+=strlen(macros[command_no]); // смещение параметров в строке
 if ((command_no >= 0) && (command_no <= 2))
  if (proc_load(command_no,param_loc,buf) == -1)
   return -1;
 if (command_no == 3)
  if (proc_write(param_loc,0,buf) == -1)
   return -1;
 if (command_no == 4)
  if (proc_read(param_loc,buf) == -1)
   return -1;
 if (command_no == 5)
  if (proc_if(param_loc,0,buf,label_max,label_loc,label_name) == -1)
   return -1;
 if (command_no == 6)
  if (proc_go_to(param_loc,buf,label_max,label_loc,label_name) == -1)
   return -1;
 if (command_no == 7)
  if (proc_for(param_loc,buf,label_max,label_loc,label_name,sub_max,sub_loc,sub_name) == -1)
   return -1;
 if (command_no == 8)
  if (proc_set(0,buf,";") == -1)
   return -1;
 if (command_no == 9)
  if ((where_i=skip(param_loc,buf,";")) == -1) return -1;
 if (command_no == 12)
  if (proc_fop_cr(param_loc,0,buf) == -1)
   return -1;
 if (command_no == 13)
  if (proc_fclose(param_loc,buf) == -1)
   return -1;
 if (command_no == 14)
  if (proc_fr_w(param_loc,0,buf) == -1)
   return -1;
 if (command_no == 15)
  if (proc_fr_w(param_loc,1,buf) == -1)
   return -1;
 if (command_no == 16)
  if (proc_fop_cr(param_loc,1,buf) == -1)
   return -1;
 if (command_no == 17)
  if (proc_fseek(param_loc,buf) == -1)
   return -1;
 if (command_no == 18)
  if (proc_rw_user(param_loc,0,buf) == -1)
   return -1;
 if (command_no == 19)
  if (proc_rw_user(param_loc,1,buf) == -1)
   return -1;
 if (command_no == 20)
  getch();
 if (command_no == 21)
  if (proc_define(param_loc,buf,var_name,var_data,def_data) == -1)
   return -1;
 if (command_no == 22)
  if (proc_filelen(param_loc,buf) == -1)
   return -1;
 if (command_no == 23)
  if (proc_rw_user(param_loc,2,buf) == -1)
   return -1;
 if (command_no == 24)
  if (proc_byte2word(param_loc,0,buf,var_name,var_data,def_data) == -1)
   return -1;
 if (command_no == 25)
  if (proc_byte2word(param_loc,1,buf,var_name,var_data,def_data) == -1)
   return -1;
 if (command_no == 26)
  if (proc_date2num(param_loc,buf,var_name,var_data,def_data) == -1)
   return -1;
 if (command_no == 27)
  if (str_len(param_loc,debug,buf,def_data,val_int,val_str,val_real)==-1)
   return -1;
 if (command_no == 29)
  if (proc_write(param_loc,1,buf) == -1)
   return -1;
 if (command_no == 31)
  clrscr();
 if (command_no == 32)
  if (proc_fr_w(param_loc,2,buf) == -1)
   return -1;
 if (command_no == 33)
  if (proc_getdate(param_loc,buf) == -1)
   return -1;
 if (command_no == 34)
  if (proc_feof(param_loc,buf) == -1)
   return -1;
 if (command_no == 35)
  if (str_find(param_loc,debug,0,buf,def_data,val_int,val_str)==-1)
   return -1;
 if (command_no == 36)
  if (str_find(param_loc,debug,1,buf,def_data,val_int,val_str)==-1)
   return -1;
 if ((where == 0) && ((command_no == 37) || (command_no == 39)))
 {
  er_in_str();
  printf("Неправильно задана главная программа!");
  return -1;
 }
 if (command_no == 38)
  if (proc_call(param_loc,buf,label_max,label_loc,label_name,sub_max,sub_loc,sub_name) == -1)
   return -1;
 
 return command_no;
}
// ════════════════════════════════════════════════════════════════════════════
void er_in_str() { printf("\n"); textcolor(RED); cprintf("Ошибка в строке %i (в команде '%s'): ",str_cur+1,macros[command_no]); }
/* ────────────────────────────────────────────────────────────────────────────
*/
char num_to_str(int i,char *string,char *temp)
{
 int m,j;

 if (string[i] == '-')
  m = var_name_size(i+1,max_var_name_size+1,string)+1;
 else
  m = var_name_size(i,max_var_name_size,string);
// m - теперь размер числа.
 for (j=0; j<m; j++,i++)      // перенесем только число в "temp_str"
  temp[j]=string[i];   // и одновременно перенесем указатель на знак
 temp[j]=0;
 return (char)temp;
}
/* ────────────────────────────────────────────────────────────────────────────
*/
char text_to_str(int i,char *string,char *temp)
{
 int m,n;
 if (string[i] == '\'')
  i++;

 m=str_size(i,string,"'");
 if (m < 0)
 {
  er_in_str();
  printf("Немогу найти конец текста!");
  return -10;
 }
 if (m > 256)
 {
  er_in_str();
  printf("Максимальный pазмеp стpоки 256 символов!");
  return -10;
 }
 for (n=0; n<m; i++,n++)
  temp[n] = string[i];
 temp[n] = 0;
 return (char)temp;
}
// ════════════════════════════════════════════════════════════════════════════
int skip(int i,char *buffer,char *one_char)
{
 i=skip_space(i,buffer);
 if (buffer[i] != one_char[0])
 {
  er_in_str();
  printf("Ошибка оформления функции, нехватает символа '%c' в позиции %i!",one_char[0],i+1);
  return -1;
 }
 else
  i++;
 i=skip_space(i,buffer);
 return i;
}
// ════════════════════════════════════════════════════════════════════════════
int check_sk(int i, char *buffer)
{
 if (buffer[i] == '"')
 {
  er_in_str();
  printf("Скобки должны быть одинарными ('), а не двойными!");
  return -1;
 }
 return 0;
}
// ════════════════════════════════════════════════════════════════════════════
int get_type(int i)
{
 if (var_data[i][0] < 3)
  return var_data[i][0];
 else
  return def_data[var_data[i][1]][1];
}
// ════════════════════════════════════════════════════════════════════════════
int get_handle(int i,char *buffer)
{
 int j;
 j=find_var(i,buffer);
 if (j == -1)
  return -1;
 if (var_data[j][0] != 0)
 {
  er_in_str();
  printf("Тип переменной ('%s'), содержащей файловый номер должен быть integer!",var_name[j]);
  return -1;
 }
 return j;
}
/* ════════════════════════════════════════════════════════════════════════════
*/
int proc_getdate(int i,char *buffer)
{
 int m,n;
 struct date d;
 getdate(&d);
 if ((i=skip(i,buffer,"(")) == -1)
  return -1;
/*
   Считаем смещение.
*/
 for (n=0; n<3; n++)
 {
/*
   Считаем переменную под год, день и месяц.
*/
  m = mem_srch(i,var_name_size(i,max_var_name_size,buffer),buffer);
  if (m != -1)
  {
   if (var_data[m][0] == 0)
   {
    if (n == 0)
     val_int[var_data[m][1]] = d.da_year;
    if (n == 1)
     val_int[var_data[m][1]] = d.da_day;
    if (n == 2)
     val_int[var_data[m][1]] = d.da_mon;
   }
   if (var_data[m][0] == 1)
   {
    if (n == 0)
     val_real[var_data[m][1]] = d.da_year;
    if (n == 1)
     val_real[var_data[m][1]] = d.da_day;
    if (n == 2)
     val_real[var_data[m][1]] = d.da_mon;
   }
   if (var_data[m][0] >= 2)
   {
    er_in_str();
    printf("Неверный тип переменной!");
    return -1;
   }
   i+=strlen(var_name[m]);
   i=skip_space(i,buffer);
  }
  if (n < 2)
   if ((i=skip(i,buffer,",")) == -1)
    return -1;
 }
 if ((i=skip(i,buffer,")")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;

 where_i=i; return 0;
}
/* ════════════════════════════════════════════════════════════════════════════
*/
int proc_fop_cr(int i,int op_cr,char *buffer)
{
 int m,n,j;
 char filename[255];
/*
   После скобки, должно идти имя файла, который нужно открыть. Имя может
   содержаться как в переменной, так и в скобках.
*/
 i=skip_first(i,buffer);
// printf("!%i!",i);
 if ((i=skip(i-1,buffer,"(")) == -1)
  return -1;
 if (check_sk(i,buffer) == -1)
  return -1;
 if (buffer[i] == '\'')
 {
/*
   Если имя содержится в скобках, переведем его во временную переменную
   "filename".
*/
  if (text_to_str(i,buffer,filename) == -10) return -1;
  i+=strlen(filename)+2; // пропустим текст и маркеры начала и конца "'".
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
  if (var_data[m][0] != 2)
  {
   er_in_str();
   printf("Тип переменной ('%s'), содержащей имя файла должен быть string!",var_name[m]);
   return -1;
  }
  strcpy(filename,val_str[var_data[m][1]]);
  i += strlen(var_name[m]); // перенесем указатель на запятую
 }
 i=skip_space(i,buffer);
 if (buffer[i] != ',')
 {
  er_in_str();
  printf("Имя открываемого файла должно отделяться от файлового номера запятой!");
  return -1;
 }
/*
   После имяни файла, через запятую должено идти имя переменной, которая
   будет содержать файловый номер. Тип переменной должен быть integer.
*/
 i=skip_space(i+1,buffer);
 if ((m=get_handle(i,buffer)) == -1)
  return -1;
 i+=strlen(var_name[m]);
 if ((i=skip(i,buffer,")")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;

 if (op_cr == 0)
  val_int[var_data[m][1]] = open(filename, O_RDWR | O_TEXT);
 if (op_cr == 1)
  val_int[var_data[m][1]] = creat(filename, S_IREAD | S_IWRITE);

 where_i=i; return 0;
}
/* ════════════════════════════════════════════════════════════════════════════
*/
int proc_rw_user(int i,int r_w,char *buffer)
{
 int m,n,iuser_no; long ruser_no; char temp_str[255];
 i=skip_first(i,buffer);
 if ((i=skip(i-1,buffer,"(")) == -1)
  return -1;
/*
   Определим имя переменной содержащей файловый номер.
*/
 if ((m=get_handle(i,buffer)) == -1)
  return -1;
 i+=strlen(var_name[m]);
/*
   Пропустим разделитель (запятую).
*/
 i=skip_space(i,buffer);
 if (buffer[i] != ',')
 {
  er_in_str();
  printf("Файловый номер должен отделяться от номера пользователя запятой!");
  return -1;
 }
 i=skip_space(i+1,buffer);
/*
   Определим имя переменной выделенной под номер пользователя.
*/
 if ((buffer[i] == '-') || ((buffer[i] >= '0') && (buffer[i] <= '9')))
 {
  num_to_str(i,buffer,temp_str);
  iuser_no=atoi(temp_str);
  ruser_no=atol(temp_str);
  i+=strlen(temp_str);
 }
 else
 {
  if ((n=find_var(i,buffer)) == -1)
   return -1;

  if ((var_data[n][0] != 0) && (var_data[n][0] != 1))
  {
   er_in_str();
   printf("Номер пользователя может содержаться только в переменной типа int или real!");
   return -1;
  }
  i+=strlen(var_name[n]);
  if (var_data[n][0] == 0)
   iuser_no=val_int[var_data[n][1]];
  else
   ruser_no=val_real[var_data[n][1]];
 }
 iuser_no--; ruser_no--;
 if (var_data[n][0] == 0)
  lseek(val_int[var_data[m][1]],iuser_no*460,SEEK_SET);
 else
  lseek(val_int[var_data[m][1]],ruser_no*460,SEEK_SET);

 if (r_w == 0)
  _read(val_int[var_data[m][1]],&stUser,sizeof(stUser));
 if (r_w == 1)
  _write(val_int[var_data[m][1]],&stUser,sizeof(stUser));
/////////////////////////////////////////////////////////
 if (r_w == 2)
 {
  if (var_data[n][0] == 0)
  {
   lseek(val_int[var_data[m][1]],iuser_no*460,SEEK_SET);
   iuser_no++;
   while (!eof(val_int[var_data[m][1]]))
   {
    lseek(val_int[var_data[m][1]],(iuser_no)*460,SEEK_SET);
    _read(val_int[var_data[m][1]],&stUser,sizeof(stUser));
    iuser_no--;
    lseek(val_int[var_data[m][1]],(iuser_no)*460,SEEK_SET);
    _write(val_int[var_data[m][1]],&stUser,sizeof(stUser));
    iuser_no+=2;
   }
  }

  if (var_data[n][0] == 1)
  {
   lseek(val_int[var_data[m][1]],ruser_no*460,SEEK_SET);
   ruser_no++;
   while (!eof(val_int[var_data[m][1]]))
   {
    lseek(val_int[var_data[m][1]],(ruser_no)*460,SEEK_SET);
    _read(val_int[var_data[m][1]],&stUser,sizeof(stUser));
    ruser_no--;
    lseek(val_int[var_data[m][1]],(ruser_no)*460,SEEK_SET);
    _write(val_int[var_data[m][1]],&stUser,sizeof(stUser));
    ruser_no+=2;
   }
  }

  _asm mov ax,4202h
  _asm mov cx,0ffffh
  _asm mov dx,0ffffh-(460-1)
  _asm int 21h
  _asm mov ah,40h
  _asm sub cx,cx
  _asm cwd
  _asm int 21h

 }
 if ((i=skip(i,buffer,")")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;

 where_i=i; return 0;
}
/* ════════════════════════════════════════════════════════════════════════════
*/
int proc_fclose(int i,char *buffer)
{
 int j;
 i=skip_first(i,buffer);
 if ((i=skip(i-1,buffer,"(")) == -1)
  return -1;
/*
   Ищем переменную содержащую файловый номер в памяти, и закрываем файл.
*/
 if ((j=get_handle(i,buffer)) == -1)
  return -1;

 close(val_int[var_data[j][1]]);
 i+=strlen(var_name[j]);
 if ((i=skip(i,buffer,")")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;

 where_i=i; return 0;
}
/* ════════════════════════════════════════════════════════════════════════════
   type: 0 - read
	 1 - write
	 2 - read string
*/
int proc_feof(int i,char *buffer)
{
 int m,n;
 i=skip_first(i,buffer);
 if ((i=skip(i-1,buffer,"(")) == -1)
  return -1;
/*
   Ищем переменную содержащую файловый номер в памяти.
*/
 if ((m=get_handle(i,buffer)) == -1)
  return -1;
 i+=strlen(var_name[m]);
 if ((i=skip(i,buffer,",")) == -1)
  return -1;
/*
   Считаем переменную под размер.
*/
 if ((n=find_var(i,buffer)) == -1)
  return -1;
 if (get_type(n) != 0)
 {
  er_in_str();
  printf("Переменная '%s' должна быть числом (integer) или указателем на число!",var_name[n]);
  return -1;
 }
/* ------------------------------ */
 if (var_data[n][0] == 0)
  val_int[var_data[n][1]] = eof(val_int[var_data[m][1]]);
 if (var_data[n][0] == 3)
 {
  void *p = (int*)(stUser.name+def_data[var_data[m][1]][0]);
  *(int *)p = eof(val_int[var_data[m][1]]);;
 }
 i+=strlen(var_name[n]);
 if ((i=skip(i,buffer,")")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;
 where_i=i; return 0;
}
/* ════════════════════════════════════════════════════════════════════════════
   type: 0 - read
	 1 - write
	 2 - read string
*/
int proc_fr_w(int i,int r_w,char *buffer)
{
 int m,n,j;
 long len;
 char temp_str[255];
 i=skip_first(i,buffer);
 if ((i=skip(i-1,buffer,"(")) == -1)
  return -1;
/*
   Определим имя переменной содержащей файловый номер.
*/
 if ((m=get_handle(i,buffer)) == -1)
  return -1;
 i+=strlen(var_name[m]);
/*
   Пропустим разделитель (запятую).
*/
 i=skip_space(i,buffer);
 if (buffer[i] != ',')
 {
  er_in_str();
  printf("Файловый номер должен отделяться от буффера запятой!");
  return -1;
 }
 i=skip_space(i+1,buffer);
/*
   Определим имя переменной выделенной под буффер.
*/
 n=find_var(i,buffer);
 if (n == -1)
  return -1;
 if (r_w == 2)
 {
  if (get_type(n) != 2)
  {
   er_in_str();
   printf("Команда предназначена для чтения строки, значит буффер может быть только типа string!");
   return -1;
  }
 }
 i+=strlen(var_name[n]);
/*
   Пропустим разделитель (запятую).
*/
 i=skip_space(i,buffer);
 if (buffer[i] != ',')
 {
  er_in_str();
  printf("Буффер должен отделяться от количества байт запятой!");
  return -1;
 }
 i=skip_space(i+1,buffer);
/*
   Определим переменную которая будет содержать количество байт для чтения.
   Функция может читать до 65.534 байт, если размер будет содержаться в
   переменной, то переменная должна быть типа real.
*/
 if ((buffer[i] == '-') || ((buffer[i] >= '0') && (buffer[i] <= '9')))
 {
  num_to_str(i,buffer,temp_str);
  len=atol(temp_str);
  i+=strlen(temp_str);
 }
 else
 {
  if ((j=find_var(i,buffer)) == -1)
   return -1;

  if (var_data[j][0] != 1)
  {
   er_in_str();
   printf("Размер может содержаться только в переменной типа real!");
   return -1;
  }
  i+=strlen(var_name[j]);
  len=val_real[var_data[j][1]];
 }
/*
   Проверим размер на "правильность!
*/
 if ((len <= 0) || (len > 65534))
 {
  er_in_str();
  printf("Немогу ");
  if (r_w == 0)
   printf("считать");
  else
   printf("записать");
  printf(" %li байт! Размер не должен быть: отрицательным числом, равняться нулю или привышать 65.534 байт!", len);
  return -1;
 }
 if ((i=skip(i,buffer,")")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;

 if (var_data[n][0] == 0)
  len=sizeof(m);
 if (var_data[n][0] == 1)
  len=sizeof(len);
 if (var_data[n][0] == 3)
 {
  if (def_data[var_data[n][1]][1] == 0)
   len=sizeof(m);
  if (def_data[var_data[n][1]][1] == 1)
   len=sizeof(len);
 }
 if (var_data[n][0] == 3)
 {
  void *p=(int*)(stUser.name+def_data[var_data[n][1]][0]);
  if (r_w == 0)
   _read(val_int[var_data[m][1]],p,(int)len);
  if (r_w == 1)
   _write(val_int[var_data[m][1]],p,(int)len);
  if (r_w == 2)
   read(val_int[var_data[m][1]],p,255);
 }
 if (var_data[n][0] == 0)
 {
  if (r_w == 0)
   _read(val_int[var_data[m][1]],&val_int[var_data[n][1]],(int)len);
  if (r_w == 1)
   _write(val_int[var_data[m][1]],&val_int[var_data[n][1]],(int)len);
 }
 if (var_data[n][0] == 1)
 {
  if (r_w == 0)
   _read(val_int[var_data[m][1]],&val_real[var_data[n][1]],(int)len);
  if (r_w == 1)
   _write(val_int[var_data[m][1]],&val_real[var_data[n][1]],(int)len);
 }
 if (var_data[n][0] == 2)
 {
  if (r_w == 0)
   _read(val_int[var_data[m][1]],&val_str[var_data[n][1]],len);
  if (r_w == 1)
   _write(val_int[var_data[m][1]],&val_str[var_data[n][1]],len);
  if (r_w == 2)
  {
   int a,b;
   long offset=tell(val_int[var_data[m][1]]);
   _read(val_int[var_data[m][1]],&val_str[var_data[n][1]],250);
   a=str_size(0,val_str[var_data[n][1]],"\x0a");
   b=(filelength(val_int[var_data[m][1]])-offset);

   if (a < b)
   {
    val_str[var_data[n][1]][a-1] = 0;
    offset+=a+1;
   }
   else
   {
    val_str[var_data[n][1]][b] = 0;
    offset+=b;
   }
   lseek(val_int[var_data[m][1]],offset,SEEK_SET);
  }
 }
 where_i=i; return 0;
}
/* ════════════════════════════════════════════════════════════════════════════
*/
int proc_fseek(int i,char *buffer)
{
 int m,n,fromwhere;
 long offset;
 char temp_str[255];

 i=skip_first(i,buffer);
 if ((i=skip(i-1,buffer,"(")) == -1)
  return -1;
/*
   Определим имя переменной содержащей файловый номер.
*/
 if ((m=get_handle(i,buffer)) == -1)
  return -1;
 i+=strlen(var_name[m]);
/*
   Пропустим разделитель (запятую).
*/
 i=skip_space(i,buffer);
 if (buffer[i] != ',')
 {
  er_in_str();
  printf("Файловый номер должен отделяться от смещения запятой!");
  return -1;
 }
 i=skip_space(i+1,buffer);
/*
   Определим имя переменной выделенной под смещение.
*/
 if ((buffer[i] == '-') || ((buffer[i] >= '0') && (buffer[i] <= '9')))
 {
  num_to_str(i,buffer,temp_str);
  offset=atol(temp_str);
  i+=strlen(temp_str);
 }
 else
 {
  if ((n=find_var(i,buffer)) == -1)
   return -1;

  if (var_data[n][0] != 1)
  {
   er_in_str();
   printf("Смещение может содержаться только в переменной типа real!");
   return -1;
  }
  offset=val_real[var_data[n][1]];
  i+=strlen(var_name[n]);
 }
/*
   Пропустим разделитель (запятую).
*/
 i=skip_space(i,buffer);
 if (buffer[i] != ',')
 {
  er_in_str();
  printf("Смещение должено отделяться от типа перехода запятой!");
  return -1;
 }
 i=skip_space(i+1,buffer);
/*
   Определим переменную которая будет содержать тип смещения.
*/
 if ((buffer[i] == '-') || ((buffer[i] >= '0') && (buffer[i] <= '9')))
 {
  num_to_str(i,buffer,temp_str);
  fromwhere=atoi(temp_str);
  i+=strlen(temp_str);
 }
 else
 {
  if ((n=find_var(i,buffer)) == -1)
   return -1;

  if (var_data[n][0] != 0)
  {
   er_in_str();
   printf("Тип перехода может содержаться только в переменной типа integer!");
   return -1;
  }
  fromwhere=val_int[var_data[n][1]];
  i+=strlen(var_name[n]);
 }
/*
   Проверим тип на "правильность!
*/
 if ((fromwhere < 0) || (fromwhere > 2))
 {
  er_in_str();
  printf("Неверный тип перемещения указателя (есть только три 0-2)!");
  return -1;
 }
 lseek(val_int[var_data[m][1]],offset,fromwhere);

 if ((i=skip(i,buffer,")")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;
 where_i=i; return 0;
}
/* ════════════════════════════════════════════════════════════════════════════
*/
int proc_filelen(int i,char *buffer)
{
 int m,n;

 i=skip_first(i,buffer);
 if ((i=skip(i-1,buffer,"(")) == -1)
  return -1;
/*
   Определим имя переменной содержащей файловый номер.
*/
 if ((m=get_handle(i,buffer)) == -1)
  return -1;
 i+=strlen(var_name[m]);
/*
   Пропустим разделитель (запятую).
*/
 i=skip_space(i,buffer);
 if (buffer[i] != ',')
 {
  er_in_str();
  printf("Файловый номер должен отделяться от переменной для хранения длины файла!");
  return -1;
 }
 i=skip_space(i+1,buffer);
/*
   Определим имя переменной выделенной для хранения длины файла.
*/
 if ((n=find_var(i,buffer)) == -1)
  return -1;
 if ((var_data[n][0] != 0) && (var_data[n][0] != 1))
 {
  er_in_str();
  printf("Длина может содержаться только в переменной типа integer или real!");
  return -1;
 }
  i+=strlen(var_name[n]);
 if (get_type(n) == 0)
  val_int[var_data[n][1]] = filelength(val_int[var_data[m][1]]);
 else
  val_real[var_data[n][1]] = filelength(val_int[var_data[m][1]]);

 if ((i=skip(i,buffer,")")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;
 where_i=i; return 0;
}
/* ════════════════════════════════════════════════════════════════════════════
*/
int proc_go_to(int i,char *buffer,int label_max,int label_loc[100],char label_name[100][20])
{
 int j;
 i=skip_space(i,buffer);

 j = label_srch(i,str_size(i,buffer,";"),buffer,label_max,label_name);
 if (j < 0)
 {
  er_in_str();
  printf("Метка/подпрограмма '");
  display_buf(i,str_size(i,buffer,";"),buffer);
  printf("' не найдена!");
  return -1;
 }
 i+=strlen(label_name[j]);
 if ((i=skip(i,buffer,";")) == -1)
  return -1;

 str_cur=label_loc[j]-1;
 where_i=i; return 0;
}
/* ════════════════════════════════════════════════════════════════════════════
*/
int proc_call(int i,char *buffer,int label_max,int label_loc[100],char label_name[100][20],int sub_max,int sub_loc[100],char sub_name[100][20])
{
 int m,n,j=str_cur;
 int temp_var_no[3]; // временные указатели на свободные номера переменных
 int temp_var_name_no=var_name_no;

 for (m=0; m<3; m++)
  temp_var_no[m]=var_no[m];
 if (proc_go_to(i,buffer,sub_max,sub_loc,sub_name) == -1)
  return -1;
 i=where_i;

 for (str_cur+=2; ; str_cur++)
 {
  n=process_command(1,label_max,label_loc,label_name,sub_max,sub_loc,sub_name);
  if (n == -1)
   return n;
  if (n == 39)
   break;
 }

 str_cur=j; var_name_no=temp_var_name_no;

 for (m=0; m<3; m++)
  var_no[m]=temp_var_no[m];

 where_i=i; return 0;
/*
   Зесь устанавливать where_i не обязательно, тк значение будет установлено
   командой go to, которая вызвается для перехода к коду подпрограммы.
   Нужно просто запомнить его значение, так как после выполнения подпрограммы
   значение обязательно испортиться.
*/
}
/* ════════════════════════════════════════════════════════════════════════════
*/
int proc_for(int i,char *buffer,int label_max,int label_loc[100],char label_name[100][20],int sub_max,int sub_loc[100],char sub_name[100][20])
{
 int m,n,j,k=0;
 int if_r[2];
 int for_start;          //для хранения строки в которой будет начинаться цикл.
 char temp_char[1], temp_for[255], temp_for_set[255];
 i=skip_space(i,buffer);
 if ((i=skip(i-1,buffer,"(")) == -1)
  return -1;
/*
   Указатель i смотрит на область присвоения начальных значений переменным.
   Если переменных несколько, они должны быть разделены запятыми.
   Ограничитель области - ";".
*/
 i=skip_space(i,buffer);
 if (buffer[i] != '#')
 {
  i=set_many(i,str_cur,buffer,",;"); // установим начальные значения
  if (i <= 0)
   return -1;                // выйдем по ошибке
 }

 n=str_size(i,buffer,";");  // n - размер области условия (область 2)
 if (n <= 0)
 {
  er_in_str();
  printf("Строка условий в цикле FOR, должна заканчиваться на ';'!");
  return -1;
 }
/*
   Вазмер входит символы ограничители (";") 1вой и 2рой областей.
   В дальнейшем n будет хранить указатель на 3тью область.
*/
 n+=i+1;
 m=skip_space(i+1,buffer); // расположение данных сравнения
 str_cur++;
 for_start=str_cur;      // строка - начало цикла
 strcpy(temp_for,buf);
 for (; ; )
 {
  if_yes_no(m,if_r,temp_for);
  if (if_r[1] == 0)
   break;
  else
   str_cur=for_start;
  for (; ; str_cur++)
  {
   j=process_command(where,label_max,label_loc,label_name,sub_max,sub_loc,sub_name);
   if (j == -1)
    return j;
   if (j == 9)
    break;
   if (j == 30)
    k = 1;
  }
  if (k == 1)
   break;
  if (buffer[n] != '#')
   if (set_many(n,for_start-1,temp_for,",)") == -1) // пройдемся по 3тьей области
    return -1;
 }
/*
   Ищем комманду завершающую выполнение цикла - "end_for" и переходим на
   строку расположенную за этой коммандой.
*/
 return command_no = 100;
}
/* ════════════════════════════════════════════════════════════════════════════
   set_many - устанавливаем значения нескольким переменным, разделенным
	      между собой символом "end[0]", последнее присвоение кончается
	      символом "end[1]".
*/
int set_many(int i,int str_cur,char *string,char *end)
{
 int m,n;
 char temp_char[1],buffer[255];

 strcpy(buffer,string);
 for (; ; )
 {
  temp_char[0] = end[1];
  m=str_size(i,buffer,end);
  n=str_size(i,buffer,temp_char);
  if ((m < n) && (m != 0))
   temp_char[0] = end[0];

  if (proc_set(i,buffer,temp_char) == -1)
   return -1;
  i+=str_size(i,buffer,temp_char) + 1;
  if ((i > (str_loc[str_cur+1] - str_loc[str_cur])) || i > 255)
  {
   printf("!%i %i!",str_loc[str_cur+1] - str_loc[str_cur],i);
   er_in_str();
   printf("Строка слишком длинная или не ограничена '%c'!",end[1]);
   return -1;
  }
  if ((m > n) || (m == 0))
   return where_i=i;
 }
}
/* ────────────────────────────────────────────────────────────────────────────
*/
int find_var(int i,char *string)
{
 int j,m;

 j = var_name_size(i,max_var_name_size,string); // размер имяни переменной
 m = mem_srch(i,j,string);
 if (m == -1)
 {
  no_var_in_mem(i,j,string);
  return -1;
 }
 return m;
}
/* ════════════════════════════════════════════════════════════════════════════
   proc_load - загрузим переменную в "память", если при описании значение
	       переменной не будет устанавливаться - временно ее значением
	       будет являться длина имяни переменной

   on start      command_no - тип переменной (0 - integer, 1 - real ...)
		 i - байт с которого сканировать буффер
		 buffer - буффер который нужно сканировать
   on exit       0 - переменная успешно загружена, иначе ошибка
*/
int proc_load(int command_no,int i,char *temp_str)
{
 int m,n,j;

 for (j=i; ; )
 {
  j=load_one_var(command_no,j,temp_str);
  if (j < 0)
   return j;
  m=str_size(j,temp_str,",");
  n=str_size(j,temp_str,";");
  if ((n < m) || ((m == 0) && (n == 0)))
   break;
  j+=m+1;
 }
 if (set_many(i,str_cur,temp_str,",;") == -1) // установим значения переменным
  return -1;
 return where_i;
}
/*
*/
int load_one_var(int command_no,int i,char *buffer)
{
 int m,n,j;
 if (var_no[command_no] > max_var)
 {
  er_in_str();
  printf("Немогу загрузить в память переменных типа '");
  display_var_type(command_no);
  printf("' больше чем %i!",max_var);
  return -1;
 }
// ---------------------------------------------
 i=skip_space(i,buffer);
 j = var_name_size(i,max_var_name_size,buffer); // размер имяни переменной
 if ((j < 0) || (command_no > 2))
 {
  er_in_str();
  printf("Переменная '");
  display_buf(i,j,buffer);
  printf("' неправильно задана!");
  return -1;
 }
 n=mem_srch(i,j,buffer);              // проверим, не загружена ли уже
 if (n != -1)                         // в память переменная с таким же именем
 {
  er_in_str();
  printf("Переменная '%s' уже находится в памяти!",var_name[j]);
  return -1;
 }

 for (m=0; m<j; m++,i++)
  var_name[var_name_no][m] = buffer[i]; // перенесем имя загруженной переменной в массив имен
 var_data[var_name_no][0] = command_no; // тип переменной
 var_data[var_name_no][1] = var_no[command_no];
 if (debug == 1)
  fprintf(debug_file,"В память загружена переменная '%s' типа %i\n",var_name[var_name_no],command_no);
// printf("В память загружена переменная '%s' типа %i\n",var_name[var_name_no],command_no);
 var_name_no++;                         // указатель в массиве имен
 var_no[command_no]++;                  // указатель в массиве переменных
 return i;
}
/* ────────────────────────────────────────────────────────────────────────────
   proc_read - считаем в переменную(ые)

   on start      i - байт с которого сканировать буффер
		 buffer - буффер который нужно сканировать
   on exit       0 - вывод закончен успешно
*/
int proc_read(int i, char *buffer)
{
 int j,m,n; long temp;
 i=skip_space(i,buffer);
 if ((i=skip(i,buffer,"(")) == -1)
  return -1;
//----------------------------
 for (; ; )
 {
  i=skip_space(i,buffer);
  if ((i > (str_loc[str_cur+1] - str_loc[str_cur])) || i > 255) // AA 20.10.00
  {
   er_in_str();
   printf("Строка слишком длинная или не ограничена ';'!");
   return -1;
  }
  temp=find_var(i,buffer);
  if (temp == -1)
   return -1;
//----------------------------
  void *p;
  n=var_data[temp][1];
  if (var_data[temp][0] == 3)
  {
   p = (int*)(stUser.name+def_data[n][0]);
   n=def_data[n][1]; // тип
  }
  else
  {
   if (var_data[temp][0] == 0) p=&val_int[n];
   if (var_data[temp][0] == 1) p=&val_real[n];
   if (var_data[temp][0] == 2) p=val_str[n];
   n=var_data[temp][0]; // тип
  }
  if (var_data[temp][0]==0)
   scanf("%i",(int*)p);
  if (var_data[temp][0]==1)
   scanf("%li",(long*)p);
  if (var_data[temp][0]==2)
   scanf("%s",p);
//   gets((char*)p);
//---
  if (debug == 1)
   fprintf(debug_file,"Считали число в переменную %s, типа '%i' в ячейку памяти %i\n",var_name[temp],var_data[temp][0],var_data[temp][1]);

  i += strlen(var_name[temp]);
  i=skip_space(i,buffer);
  if (buffer[i] == ')')
   break;
  if (buffer[i] == ',')
   i++;
 }
 if ((i=skip(i,buffer,")")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;
 return where_i=i;
}
/* ────────────────────────────────────────────────────────────────────────────
   proc_write - выведем на экран данные

   on start      i - байт с которого сканировать буффер
		 buffer - буффер который нужно сканировать
   on exit       0 - вывод закончен успешно
*/
int proc_write(int i,int type,char *buffer)
{
 int j,m,k,n; long temp; void *pointer;
 char str[255], temp_str[255];
//
 i=skip_space(i,buffer);                    // пропустим пробелы
 if ((i=skip(i,buffer,"(")) == -1)
  return -1;
/*
   Если команда fwrite.
*/
 if (type == 1)
 {
/*
   Определим имя переменной содержащей файловый номер.
*/
 if ((k=get_handle(i,buffer)) == -1)
  return -1;
 i+=strlen(var_name[k]);
 i=skip_space(i,buffer);
 if ((i=skip(i,buffer,",")) == -1)
  return -1;
 }
/*
   Цикл вывода (на экран, если type = 0 или в файл, если type = 1)
*/
 for (; ; )
 {
  if ((i > (str_loc[str_cur+1] - str_loc[str_cur])) || i > 255) // AA 20.10.00
  {
   er_in_str();
   printf("Строка слишком длинная или не ограничена ';'!");
   return -1;
  }
  pointer=&temp_str;
  if (buffer[i] == ')')
  {
   if ((i=skip(i+1,buffer,";")) == -1) return -1;
    return where_i=i;
  }

  ///////////////////////
  if (buffer[i] == '\'')
  {
   i++;
   j=str_size(i,buffer,"'");
   if (j <= 0)
   {
    er_in_str();
    printf("Не закрыта скобка \"'\"!\n");
    return -1;
   }
   for (m=0; m<j; m++,i++)
   {
    if ((buffer[i] == '\\') && (buffer[i+1] == 'n'))
    {
     if (type == 0) printf("\n");
     if (type == 1)
     {
      n=2573; pointer=&n; // n = <cr> (13,10)
      _write(val_int[var_data[k][1]],pointer,2);
     }
     i++; m++;
    }
    else
    {
     if (type == 0) printf("%c",buffer[i]);
     if (type == 1) { pointer=&buffer[i]; _write(val_int[var_data[k][1]],pointer,1); }
    }
   }
   i++;                               // пропустим "'"
  }
//------------------------------------
  else
  {
   if (buffer[i] == ',')
    i++;

   if ((temp=find_var(i,buffer)) == -1) return -1;

   n=var_data[temp][1];
   if (var_data[temp][0] == 0)
   {
    if (type == 0) printf("%i",val_int[n]);
    if (type == 1)
    {
     itoa(val_int[n],(char*)pointer,10);
     _write(val_int[var_data[k][1]],pointer,strlen((char*)pointer));
    }
   }
   if (var_data[temp][0] == 1)
   {
    if (type == 0) printf("%li",val_real[n]);
    if (type == 1)
    {
     ltoa(val_real[n],(char*)pointer,10);
     _write(val_int[var_data[k][1]],pointer,strlen((char*)pointer));
    }
   }
   if (var_data[temp][0] == 2)
   {
    if (type == 0) printf("%s",val_str[n]);
    if (type == 1)
    {
     pointer=&val_str[n];
     _write(val_int[var_data[k][1]],pointer,strlen((char*)pointer));
    }
   }
   if (var_data[temp][0] == 3)
   {
    void *p = (int*)(stUser.name+def_data[n][0]);
    if (def_data[n][1] == 0)
    {
     if (type == 0) printf("%i",*(int*)p);
     else
     {
      itoa(*(int*)p,(char*)pointer,10);
      _write(val_int[var_data[k][1]],pointer,strlen((char*)pointer));
     }
    }
    if (def_data[n][1] == 1)
    {
     if (type == 0) printf("%li",*(long*)p);
     else
     {
      ltoa(*(long*)p,(char*)pointer,10);
      _write(val_int[var_data[k][1]],p,strlen((char*)pointer));
     }
    }
    if (def_data[n][1] == 2) // define type = string
    {
     if (type == 0) printf("%s",(char*)p);
     else
      _write(val_int[var_data[k][1]],p,strlen((char*)p));
    }
   }
   i += strlen(var_name[temp]);
   if (buffer[i] != ')')
    if ((i=skip(i,buffer,",")) == -1) return -1;
  }
 }
}
/* ════════════════════════════════════════════════════════════════════════════
   check_macro - проверим строку "buf" с символа "i" на наличие в текущей пози-
		 ции комманды "j" из списка "macros".

   on start      i - байт с которого сканировать буффер
		 j - имя комманды которую нужно искать
		 buf - буффер который нужно сканировать
   on exit       1 - комманда найдена
		 0 - комманда не найдена
*/
int check_macro(int i, int j, char *buf)
{
 int k=0;
 for (; k<strlen(macros[j]); i++,k++)
  if (macros[j][k] != buf[i])
   return 0;
 return 1;
}
/* ────────────────────────────────────────────────────────────────────────────
   scan_data - проверим строку "buffer" размером "length". если комманда найде-
	       на, переменная "command_no" будет содержать номер позиции.

   on start      length - длина проверяемой строки
		 buffer - проверяемая строка
   on exit    -1 - комманда не найдена
	      -2 - ошибка
	      %d - смещение начала комманды в строке
*/
int scan_data(char *buffer,int length)
{
 int i,j,k;
 i=skip_space(0,buffer);
 if (i > length)
  return -1;

 if (buffer[i] == ';')
 {
  command_no=28;
  return i;
 }

 for (j=0; j<max_macros; j++)
 {
  if (j == 8)
  {
   for (k=i; k<length; k++)
   {
    if (buffer[k] == '\'')
     break;
    else
     if (buffer[k] == '=')
     {
      command_no=j;
      return k;
     }
   }
  }
  if (check_macro(i,j,buffer) == 1)
  {
   command_no=j;
   return i;
  }
 }
 return -1;
}
/* ════════════════════════════════════════════════════════════════════════════
   cmp_var - сверяет строку с i позиции с именем m переменной

   on start  i - номер позиции строки
	     m - номер переменной
	     buffer - строка
   on exit   0 - имя переменной совпало, иначе нет
*/
int cmp_var(int i, int m, char *buffer)
{
 int j;
 for (j=0; j<strlen(var_name[m]); i++,j++)
  if (buffer[i] != var_name[m][j])
   return -1;
 return 0;
}
/* ────────────────────────────────────────────────────────────────────────────
   mem_srch - ищет в "памяти" (массиве) переменную (размер имяни j)
	      из строки buffer, символа строки i

   on start  i - номер позиции строки
	     j - размер имяни переменной
	     buffer - строка
   on end    -1 если в памяти нет такой переменной, иначе возвращает номер
	     найденой переменной
*/
int mem_srch(int i, int j, char *buffer)
{
 int m;
 if ((var_name_no == 0))
  return -1;                       // если ни одной переменной небыло загружено
 for (m=0; m<var_name_no; m++) // цикл проверки всех загруженных переменных типа command_no
 {
  if (j == strlen(var_name[m]))    // если длина переменных не совпадает
   if (cmp_var(i,m,buffer) == 0)
    return m;                      // если имена совпали, вернем номер
 }
 return -1;                        // если произошла ошибка
}
/* ────────────────────────────────────────────────────────────────────────────
   label_srch - ищет в "памяти" (массиве) метку (размер имяни j)
	      из строки buffer, символа строки i

   on start  i - номер позиции строки
	     j - размер имяни переменной
	     buffer - строка
   on end    -1 если в памяти нет такой метки, иначе возвращает номер
	     в базе label_name.
*/
int label_srch(int i, int j, char *buffer,int label_max,char label_name[100][20])
{
 int m;
 if ((label_max == 0))
  return -1;                       // если ни одной метки небыло загружено
 for (m=0; m<label_max; m++)       // цикл проверки всех загруженных меток
 {
  if (j == strlen(label_name[m]))  // если длина меток совпадает
   if (cmp_lab(i,m,buffer,label_name) == 0)
    return m;                      // если имена совпали, вернем номер
 }
 return -1;                        // если произошла ошибка
}
int cmp_lab(int i, int m, char *buffer,char label_name[100][20])
{
 int j;
 for (j=0; j<strlen(label_name[m]); i++,j++)
  if (buffer[i] != label_name[m][j])
   return -1;
 return 0;
}
/*
*/
void no_var_in_mem(int i, int j, char *buffer)
{
 er_in_str();
 printf("Переменная '");
 display_buf(i,j,buffer);
 printf("' не найдена в памяти!");
}
