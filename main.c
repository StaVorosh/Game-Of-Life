#include"header.h"

int main(int argc, char **argv) {
    int rank, size, m, n, local_m, local_n, last_add;
    int play;
	char file[20];
    int Recv_size, Send_size;//размеры принимаемых и отправляемых сообщений
    int Gens;//количество поколений
    int dst, src, tag;
	int *Local_World, *Global_World, *Tmp_Buf;
	int i,j,k,l;
	MPI_Status status;
	
	// подключение mpi
    MPI_Init(&argc, &argv);

	// Узнаем ранк процессора и количество процессоров
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);    

	
	Get_Data(&m,&n,&Gens,rank);//считываем входные данные (размеры игрового поля m - количество строк, n - столбцы)
	
	if(rank==0)
	{
		
		Global_World=Mem_Alloc(m+2,n+2);//выделение памяти под всю таблицу
		
		Init_World(Global_World,"input.txt", m, n);//заполнение таблицы
		
		//Draw_World("test1.txt",Global_World,m+2,n+2);
		
		Distrib(Global_World,m,n,size);//рассылка каждому процессору своей рабочей области (кроме нулевого естественно)
		
		local_n=n/size+2;
		local_m=m+2;
		
		Local_World=Mem_Alloc(local_m,local_n);
		
		for(i=0;i<local_m;i++)
			for(j=0;j<local_n;j++)
				Local_World[i*local_n+j]=Global_World[i*(n+2)+j];
		//Draw_World("test2.txt",Local_World,local_m,local_n);
		for(k=0;k<Gens;k++)
		{
			play=1;
			for(l=1;l<size;l++)
				MPI_Send(&play,1,MPI_INT,l,l,MPI_COMM_WORLD);
			
			Life(Local_World,local_m,local_n);
			
			//Draw_World("test3.txt",Local_World,local_m,local_n);
			
			Tmp_Buf=Mem_Alloc(local_m,1);//выделение памяти под массив для обменов	
			Send_size=local_m;			
			Recv_size=local_m;
			
			/* прием последнего столбца с последнего процессора на 0 */
			Receive(&src,&tag,size-1,0);
			MPI_Recv(Tmp_Buf,Recv_size,MPI_INT,src,tag,MPI_COMM_WORLD,&status);
			
			for(i=0;i<local_m;i++)
				Local_World[i*local_n]=Tmp_Buf[i];
			/* -------------------------------------------------------- */
			
			/* отправка последнего столбца из 0 процессора на 1 */
			for(i=0;i<local_m;i++)
				Tmp_Buf[i]=Local_World[i*local_n+local_n-2];
				
			Send(&dst,&tag,1,0);
			MPI_Send(Tmp_Buf,Send_size,MPI_INT,dst,tag,MPI_COMM_WORLD);
			/* -------------------------------------------------------- */	
			
			/* прием первого столбца с 1 процессора на 0 */
			
			Receive(&src,&tag,1,0);
			MPI_Recv(Tmp_Buf,Recv_size,MPI_INT,src,tag,MPI_COMM_WORLD,&status);
			
			for(i=0;i<local_m;i++)
				Local_World[i*local_n+local_n-1]=Tmp_Buf[i];				
			/* -------------------------------------------------------- */
			
			/* отправка 1 столбца в 0 процессоре на последний процессор */
			for(i=0;i<local_m;i++)
				Tmp_Buf[i]=Local_World[i*local_n+1];
			Tmp_Buf[0]=Local_World[0]; //возьня с уголками
			Tmp_Buf[local_m-1]=Local_World[(local_m-1)*local_n];
			
			Send(&dst,&tag,size-1,0);
			MPI_Send(Tmp_Buf,Send_size,MPI_INT,dst,tag,MPI_COMM_WORLD);
			/* -------------------------------------------------------- */		
			free(Tmp_Buf);
		}
		play=0;
		for(l=1;l<size;l++)//рассылка этого флага остальным процессорам
		{
			Send(&dst,&tag,l,l);
			MPI_Send(&play,1,MPI_INT,dst,tag,MPI_COMM_WORLD);
		}

		for(i=0;i<local_m;i++)
			for(j=0;j<local_n;j++)
				Global_World[i*(n+2)+j]=Local_World[i*local_n+j];
		free(Local_World);
		
		//Draw_World("test_f.txt",Global_World,m+2,n+2);
		
		for(k=1;k<size;k++)//прием с каждого процессора соответствуещей части от всего игрового поля
		{
			if(k==size-1)
				last_add=n%size;
			else
				last_add=0;
			
			local_n+=last_add;

			Local_World=Mem_Alloc(local_m,local_n);			

			Recv_size=local_m*local_n;
				
			Receive(&src,&tag,k,k+1);
			MPI_Recv(Local_World,Recv_size,MPI_INT,src,tag,MPI_COMM_WORLD,&status);

			for(i=0;i<local_m;i++)
				for(j=0;j<local_n;j++)
					Global_World[i*(n+2)+(j+(local_n-2)*k)]=Local_World[i*local_n+j];

			free(Local_World);
		}
		Draw_World("test_final.txt",Global_World,m+2,n+2);
		free(Global_World);		
	}
	else
	{
		local_m=m+2;
		if(rank!=size-1)
			local_n=n/size+2;
		else
			local_n=n/size+2+n%size;

		Recv_size=local_m*local_n;

		Local_World=Mem_Alloc(local_m,local_n);
		
		Receive(&src,&tag,0,rank);
		MPI_Recv(Local_World,Recv_size,MPI_INT,src,tag,MPI_COMM_WORLD,&status);//прием соответствующей части от всего игрового поля
		
		//Draw_World("testiyuiasdfhjas.txt",Local_World,local_m,local_n);

		Tmp_Buf=Mem_Alloc(local_m,1);

		Recv_size=local_m;
		Send_size=local_m;
		
		Receive(&src,&tag,0,rank);
		MPI_Recv(&play,1,MPI_INT,src,tag,MPI_COMM_WORLD,&status);//прием флага play
		
		while(play)
		{
			Life(Local_World,local_m,local_n);
			//myprint("test4.txt",play);

			if(rank>0 && rank<size-1)
			{
				/* прием последнего столбца с предыдущего процессора на текущий */
				Receive(&src,&tag,rank-1,0);
				MPI_Recv(Tmp_Buf,Recv_size,MPI_INT,src,tag,MPI_COMM_WORLD,&status);
				for(i=0;i<local_m;i++)
					Local_World[i*local_n]=Tmp_Buf[i];
				/* -------------------------------------------------------- */
				
				/* отправка последнего столбца из текущего процессора на следующий */
				for(i=0;i<local_m;i++)
					Tmp_Buf[i]=Local_World[i*local_n+local_n-2];
				
				Send(&dst,&tag,rank+1,0);
				MPI_Send(Tmp_Buf,Send_size,MPI_INT,dst,tag,MPI_COMM_WORLD);
				/* -------------------------------------------------------- */
				
				/* прием первого столбца со следующего процессора на текущий */

				Receive(&src,&tag,rank+1,0);
				MPI_Recv(Tmp_Buf,Recv_size,MPI_INT,src,tag,MPI_COMM_WORLD,&status);

				for(i=0;i<local_m;i++)
					Local_World[i*local_n+local_n-1]=Tmp_Buf[i];
				/* -------------------------------------------------------- */
				
				/* отправка первого столбца из текущего процессора на предыдущий */
				for(i=0;i<local_m;i++)
					Tmp_Buf[i]=Local_World[i*local_n+1];
					
				Send(&dst,&tag,rank-1,0);
				MPI_Send(Tmp_Buf,Send_size,MPI_INT,dst,tag,MPI_COMM_WORLD);
				/* -------------------------------------------------------- */
			}
			if(rank==size-1)
			{
				/* отправка последнего столбца из последнего процессора на 0 */
				for(i=0;i<local_m;i++)
					Tmp_Buf[i]=Local_World[i*local_n+local_n-2];
				Tmp_Buf[0]=Local_World[local_n-1];
				Tmp_Buf[local_m-1]=Local_World[(local_m-1)*local_n+(local_n-1)];

				Send(&dst,&tag,0,0);
				MPI_Send(Tmp_Buf,Send_size,MPI_INT,dst,tag,MPI_COMM_WORLD);
				/* -------------------------------------------------------- */

				/* прием последнего столбца с предпоследнего процессора на последний (текущий) */

				Receive(&src,&tag,rank-1,0);
				MPI_Recv(Tmp_Buf,Recv_size,MPI_INT,src,tag,MPI_COMM_WORLD,&status);
				for(i=0;i<local_m;i++)
					Local_World[i*local_n]=Tmp_Buf[i];
				/* -------------------------------------------------------- */

				/* отправка первого столбца из последнего процессора на предпоследний */
				for(i=0;i<local_m;i++)
					Tmp_Buf[i]=Local_World[i*local_n+1];
					
				Send(&dst,&tag,rank-1,0);
				MPI_Send(Tmp_Buf,Send_size,MPI_INT,dst,tag,MPI_COMM_WORLD);
				/* -------------------------------------------------------- */				
				
				/* прием первого столбца с 0 процессора на последний */
				
				Receive(&src,&tag,0,0);
				MPI_Recv(Tmp_Buf,Recv_size,MPI_INT,src,tag,MPI_COMM_WORLD,&status);
				for(i=0;i<local_m;i++)
					Local_World[i*local_n+local_n-1]=Tmp_Buf[i];
				/* -------------------------------------------------------- */				
				
			}
			Receive(&src,&tag,0,rank);
			MPI_Recv(&play,1,MPI_INT,src,tag,MPI_COMM_WORLD,&status);//прием флага play
		}
		//Draw_World("test_piska.txt",Local_World,local_m,local_n);

		Send_size=local_n*local_m;
		Send(&dst,&tag,0,rank+1);
		MPI_Send(Local_World,Send_size,MPI_INT,dst,tag,MPI_COMM_WORLD);//отправка соответствующей части на 0 процессор

		free(Tmp_Buf);
	}  
    MPI_Finalize();

    return 0;
}

//дебажный вывод
void myprint(char *filename, int a)
{
	FILE *fi;
	fi=fopen(filename,"a");
	fprintf(fi,"%d\n",a);
	fclose(fi);
	return;
}

void Distrib(int *Global_World, int m, int n, int size)
{
	int local_m, local_n, last_add = 0;//размер рабочей области, last_add - дополнительное количество столбцов в последней
	int i,j,k, dst, tag;
	int Send_size;//количество элементов в рабочей области
	int* Local_World;
	
	local_m = m + 2;
	local_n = n/size + 2;
	for(k=1;k<size;k++)
	{
		if(k == size-1)
			last_add = n%size;
		local_n += last_add;

		Local_World=Mem_Alloc(local_m,local_n);			

		for(i=0;i<local_m;i++)
			for(j=0;j<local_n;j++)
				Local_World[i*local_n+j]=Global_World[i*(n+2)+(j+(local_n-2)*k)];

		Send_size = local_m*local_n;
		Send(&dst,&tag,k,k);
		MPI_Send(Local_World,Send_size,MPI_INT,dst,tag,MPI_COMM_WORLD);

		free(Local_World);
	}	
}

void Get_Data(int *m, int *n, int *Gens, int rank)
{
	if(rank == 0)
	{
		*n = widht;
		*m = height;
		*Gens = generations;
	}
	MPI_Bcast(m,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(n,1,MPI_INT,0,MPI_COMM_WORLD);
    return;
}

void Receive(int *src, int *tag, int n_src, int n_tag)
{
	*src = n_src;
	*tag = n_tag;
	return;
}

void Send(int *dst, int *tag, int n_dst, int n_tag)
{
	*dst = n_dst;
	*tag = n_tag;
	return;
}

int* Mem_Alloc(int m, int n)
{
	int *world;
	world=(int*)malloc(m*n*sizeof(int));

	if(!world)
	{
		printf("Memory not allocated!\n");
		MPI_Abort(MPI_COMM_WORLD,1);
	}

	return world;
}
	
void Init_World(int* World, char* filename, int m, int n)

{

	FILE *fi;

	int i,j,k,tmp;

	fi=fopen(filename,"r");
	if(!fi)
	{
		printf("Error opening datafile!\n");
		MPI_Abort(MPI_COMM_WORLD,2);
	}

	//fseek(fi,(2)*sizeof(int),SEEK_SET);
	k=n+2;
	for(i=1;i<m+1;i++)
	{
		for(j=1;j<n+1;j++)
		{
			if(fscanf(fi,"%d",&tmp))
				World[i*k+j]=tmp;
			else
			{
				printf("Error world initialization!\n");
				MPI_Abort(MPI_COMM_WORLD,4);
			}				
		}
	}		
	for(i=1;i<m+1;i++)
	{
		World[i*k]=World[i*k+n];
		World[i*k+(n+1)]=World[i*k+1];
	}
	for(j=1;j<n+1;j++)
	{
		World[j]=World[m*k+j];
		World[(m+1)*k+j]=World[k+j];
	}
	World[0]=World[m*k+n];
	World[(m+1)*k]=World[k+n];
	World[n+1]=World[m*k+1];
	World[(m+1)*k+n+1]=World[k+1];

	fclose(fi);

}

int Neigh_Value(int* World, int i, int j, int n)
{
	if(World[i*n+j]==1)
		return 1;
	else
		return 0;
}


int Cnt_Neighbors(int* world, int i, int j, int n)
{
	int neighbors=0;
	neighbors+=Neigh_Value(world,i-1,j-1,n);
	neighbors+=Neigh_Value(world,i-1,j,n);
	neighbors+=Neigh_Value(world,i-1,j+1,n);
	neighbors+=Neigh_Value(world,i,j-1,n);
	neighbors+=Neigh_Value(world,i,j+1,n);
	neighbors+=Neigh_Value(world,i+1,j-1,n);
	neighbors+=Neigh_Value(world,i+1,j,n);
	neighbors+=Neigh_Value(world,i+1,j+1,n);
	return neighbors;
}	
	

void Life(int *World, int m, int n)
{
	int Neighbors;
	int *World_New;
	int i,j;
	
	World_New=Mem_Alloc(m,n);
		
	for(i=1;i<m-1;i++)
	{
		for(j=1;j<n-1;j++)
		{
			Neighbors=Cnt_Neighbors(World,i,j,n);
			if(Neighbors==3)
				World_New[i*n+j]=1;
			else if(Neighbors==2 && World[i*n+j]==1)
				World_New[i*n+j]=1;
			else
				World_New[i*n+j]=0;
		}
	}
	for(i=1;i<m-1;i++)
		for(j=1;j<n-1;j++)
			World[i*n+j]=World_New[i*n+j];
	
	//обмен нижней и верхней строкой в рабочем блоке
	for(j=1;j<n-1;j++)
	{
		World[j]=World[(m-2)*n+j];
		World[(m-1)*n+j]=World[n+j];
	}	
	
	free(World_New);
}

void Draw_World(char *filename, int *World, int m, int n)
{
	int i, j;
	FILE *fo;
	fo=fopen(filename,"a");
	for (i = 1; i < m-1; i++) 
	{
		for (j = 1; j < n-1; j++)
			fprintf(fo,World[i*n+j] > 0 ? "1" : "0");
		fprintf(fo,"\n");
	}
	fprintf(fo,"\n");
	fclose(fo);
}