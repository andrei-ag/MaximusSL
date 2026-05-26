// ════════════════════════════════════════════════════════════════════════════
int proc_define(int i,char *buffer,char var_name[400][20],int var_data[400][2],int def_data[100][2])
{
 int m,n,k;
 char temp_str[255];
 if (cur_def > max_var)
 {
  er_in_str();
  printf("Немогу загрузить в память данных о пользователе больше чем %i!",max_var);
  return -1;
 }
// ---------------------------------------------
 i=skip_space(i,buffer);
 m = var_name_size(i,max_var_name_size,buffer); // размер имяни переменной
 if (m < 0)
 {
  er_in_str();
  printf("Переменная '");
  display_buf(i,m,buffer);
  printf("' неправильно задана!");
  return -1;
 }
 n=mem_srch(i,m,buffer);              // проверим, не загружена ли уже
 if (n != -1)                         // в память переменная с таким же именем
 {
  er_in_str();
  printf("Переменная '%s' уже находится в памяти!",var_name[n]);
  return -1;
 }

 for (k=0; k<m; k++,i++)
  var_name[var_name_no][k] = buffer[i]; // перенесем имя загруженной переменной в массив имен
 var_name[var_name_no][k] = 0;
 var_data[var_name_no][0] = 3; // тип переменной "define"
 var_data[var_name_no][1] = cur_def;
 if (debug == 1)
  fprintf(debug_file,"В память загружены данные о пользователе ('%s'): ",var_name[var_name_no]);
 if ((i=skip(i,buffer,"[")) == -1)
  return -1;
//
 if ((buffer[i] == '-') || ((buffer[i] >= '0') && (buffer[i] <= '9')))
 {
  num_to_str(i,buffer,temp_str);
  def_data[cur_def][0]=atoi(temp_str);
  i+=strlen(temp_str);
 }
 else
 {
  if ((m=find_var(i,buffer)) == -1)
   return -1;

  if (var_data[m][0] != 0)
  {
   er_in_str();
   printf("Смещение может содержаться только в переменной типа int!");
   return -1;
  }
  def_data[cur_def][0]=val_int[var_data[m][1]];
  i+=strlen(var_name[m]);
 }
 if (debug == 1)
  fprintf(debug_file,"смещение %i; ",def_data[cur_def][0]);
//
 if ((i=skip(i,buffer,",")) == -1)
  return -1;
//
 if ((buffer[i] == '-') || ((buffer[i] >= '0') && (buffer[i] <= '9')))
 {
  num_to_str(i,buffer,temp_str);
  i+=strlen(temp_str);
  if ((def_data[cur_def][1]=atoi(temp_str)) > 2)
  {
   def_er:
   er_in_str();
   printf("Неправильный тип (%i), тип может быть числом от 0 до 2!",def_data[cur_def][1]);
   return -1;
  }
 }
 else
  goto def_er;
 if (debug == 1)
  fprintf(debug_file,"тип %i;\n",def_data[cur_def][1]);
 if ((i=skip(i,buffer,"]")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;

 var_name_no++;                         // указатель в массиве имен
 cur_def++;                             // указатель в массиве переменных
 return i;
}
/* ════════════════════════════════════════════════════════════════════════════
   type: 0 - из байта, указанного в смещении сделать integer и положить в
	 другую переменную
	 1 - взять значени переменной, переделать в byte и положить по
	 адресу указанного смещения

*/
int proc_date2num(int i,char *buffer,char var_name[400][20],int var_data[400][2],int def_data[100][2])
{
 int m,n;
 if ((i=skip(i,buffer,"(")) == -1)
  return -1;
/*
   Считаем смещение.
*/
 if ((m=find_var(i,buffer)) == -1)
  return -1;

 void *p = (int*)(stUser.name+def_data[var_data[m][1]][0]);
 if ((var_data[m][0] != 3) || ((var_data[m][0] == 3) && (def_data[var_data[m][1]][1] >= 2)))
 {
  er_in_str();
  printf("Неверный тип переменной под смещение!");
  return -1;
 }
 st=*(_dos_stamp *)p;
 i+=strlen(var_name[m]);
//
 for (n=0; n<3; n++)
// for (n=0; n<6; n++)
 {
  if ((i=skip(i,buffer,",")) == -1)
   return -1;
/*
   Считаем переменную под день.
*/
  m = mem_srch(i,var_name_size(i,max_var_name_size,buffer),buffer);
  if (m != -1)
  {
   if (var_data[m][0] == 0)
   {
    if (n == 0)
     val_int[var_data[m][1]] = st.date.da;
    if (n == 1)
     val_int[var_data[m][1]] = st.date.mo;
    if (n == 2)
     val_int[var_data[m][1]] = st.date.yr;
    if (n == 3)
     val_int[var_data[m][1]] = st.time.ss;
    if (n == 4)
     val_int[var_data[m][1]] = st.time.mm;
    if (n == 5)
     val_int[var_data[m][1]] = st.time.hh;
   }
   if (var_data[m][0] == 1)
   {
    if (n == 0)
     val_real[var_data[m][1]] = st.date.da;
    if (n == 1)
     val_real[var_data[m][1]] = st.date.mo;
    if (n == 2)
     val_real[var_data[m][1]] = st.date.yr;
    if (n == 3)
     val_real[var_data[m][1]] = st.time.ss;
    if (n == 4)
     val_real[var_data[m][1]] = st.time.mm;
    if (n == 5)
     val_real[var_data[m][1]] = st.time.hh;
   }
   if (var_data[m][0] >= 2)
   {
    er_in_str();
    printf("Неверный тип переменной!");
    return -1;
   }
   if (debug == 1)
    fprintf(debug_file,"Преводим в переменную %s типа %i\n",var_name[m],var_data[m][0]);
   i+=strlen(var_name[m]);
   i=skip_space(i,buffer);
  }
  else
  {
   if (debug == 1)
    fprintf(debug_file,"Не переводим (пропустим).\n");
   i+=str_size(i,buffer,",");
  }
 }
 if ((i=skip(i,buffer,")")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;
 return 0;
}
/* ════════════════════════════════════════════════════════════════════════════
   type: 0 - из байта, указанного в смещении сделать integer и положить в
	 другую переменную
	 1 - взять значени переменной, переделать в byte и положить по
	 адресу указанного смещения

*/
int proc_byte2word(int i,int type,char *buffer,char var_name[400][20],int var_data[400][2],int def_data[100][2])
{
 int m;
 if ((i=skip(i,buffer,"(")) == -1)
  return -1;
/*
   Считаем смещение.
*/
 if ((m=find_var(i,buffer)) == -1)
  return -1;

 void *p = (int*)(stUser.name+def_data[var_data[m][1]][0]);
 if ((var_data[m][0] != 3) || ((var_data[m][0] == 3) && (def_data[var_data[m][1]][1] >= 2)))
 {
  er_in_str();
  printf("Неверный тип переменной под смещение!");
  return -1;
 }
 i+=strlen(var_name[m]);
 if ((i=skip(i,buffer,",")) == -1)
  return -1;
/*
   Считаем имя переменной под число.
*/
 if ((m=find_var(i,buffer)) == -1)
  return -1;

 if (type == 0)  // из байта в int или real
 {
  if (var_data[m][0] == 0)
   val_int[var_data[m][1]]=*(byte *)p;
  if (var_data[m][0] == 1)
   val_real[var_data[m][1]]=*(byte *)p;
  if (var_data[m][0] >= 2)
  {
   er_in_str();
   printf("Немогу перевести байт в этот тип переменной!");
   return -1;
  }
 }
 if (type == 1) // из int в байт по смещению
 {
  if (var_data[m][0] == 0)
   *(byte *)p=val_int[var_data[m][1]];
  if (var_data[m][0] >= 1)
  {
   er_in_str();
   printf("Немогу перевести в байт из этого типа переменной!");
   return -1;
  }
 }
 i+=strlen(var_name[m]);
 if ((i=skip(i,buffer,")")) == -1)
  return -1;
 if ((i=skip(i,buffer,";")) == -1)
  return -1;
 return 0;
}
