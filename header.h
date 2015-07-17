#include<stdio.h>
#include<stdlib.h>
#include"mpi.h"
#define widht 10         //������ �������� ����
#define height 10        //������ �������� ����
#define generations 10   //���������� ���������

void Get_Data(int*, int*, int*, int);
void Receive(int *, int *, int, int);
void Send(int *, int *, int, int);
int* Mem_Alloc(int, int);
void Distrib(int *, int, int, int);
void Init_World(int*, char*, int, int);
int Neigh_Value(int*, int, int, int);
int Cnt_Neighbors(int*, int, int, int);
void Life(int*, int, int);
void Draw_World(char*, int *, int, int);
void myprint(char*, int);