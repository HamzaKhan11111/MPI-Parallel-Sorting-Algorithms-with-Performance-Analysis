#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include<fstream>
#include <cmath>
#include<iostream>
#include <time.h>
#include <sys/time.h>
#include <string>
using std::ofstream;
using namespace std;
ofstream out;
ofstream t;
ofstream initial;

int operation_count=0;
struct leaf
{
	int s;
	int end=0;
	int *arr;
};
void merge(int arr[],int left,int mid,int right)
{
	int n1=mid-left+1;
	int n2=right-mid;
	
	int l[n1];
	int r[n2];
	
	for(int i=0;i<n1;i++)
	{
		l[i]=arr[left+i];
		operation_count++;
	}
	for(int j=0;j<n2;j++)
	{
		r[j]=arr[mid+1+j];
		operation_count++;
	}
		
	int i=0;
	int j=0;
	int k=left;
	
	while(i<n1 && j<n2)
	{
		if(l[i]<=r[j])
		{
			arr[k]=l[i];
			operation_count++;
			i++;
		}
		else
		{
			arr[k]=r[j];
			operation_count++;
			j++;
		}
		k++;
	}
	
	while(i<n1)
	{
		arr[k]=l[i];
		operation_count++;
		i++;
		k++;
	}
	while(j<n2)
	{
		arr[k]=r[j];
		operation_count++;
		j++;
		k++;
	}
}

void mergeSort(int arr[],int l,int r)
{
	if(l<r)
	{
		int mid=l+(r-l)/2;
		mergeSort(arr,l,mid);
		mergeSort(arr,mid+1,r);
		merge(arr,l,mid,r);
	}
}

int partition(int arr[], int low, int high) 
{
	int pivot=arr[high];
	int i=low-1;
	for (int j=low;j<=high-1;j++) 
	{
		if (arr[j]<pivot) 
		{
			operation_count++;
			i++;
			int temp=arr[i];
			arr[i]=arr[j];
			arr[j]=temp;
		}
		operation_count++;
	}
	int temp = arr[i+1];
	arr[i+1] = arr[high];
	arr[high] = temp;
	return i+1;
}

void quickSort(int arr[],int low,int high)
{
	if (low<high) 
	{
		int pivot = partition(arr,low,high);
		quickSort(arr,low, pivot-1);
		quickSort(arr,pivot+1,high);
	}
}


int main(int argc, char **argv)
{
	int buffer_size;
	int rank=0; 
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
	{
		out.open("output.txt");
	    	t.open("time.txt");
	    	initial.open("inital.txt");
		int size=0;
		
		printf("Input size of array(11,13,15,17)\n");
		cin>>size;
		
		int choice;
		printf("choose 1 for merge and 2 for quick sort\n");
		cin>>choice;
		
		size=pow(2,size);
		
		int* arr=(int*) malloc(size*sizeof(int));
		if(choice==1)
			initial<<"Merge Sort of size "<<size<<":"<<endl;
		else	
			initial<<"Quick Sort of size "<<size<<":"<<endl;
		for(int i=0;i<size;i++)
		{
			arr[i]=(rand()%1000);
			initial<<arr[i]<<" ";
		}
		leaf l[3];
		int k=0;
		for(int i=0;i<3;i++)
		{
			l[i].s=size/3;
			l[i].end=(size/3);
			l[i].arr=new int[size/3];
			for(int j=0;j<l[i].end;j++)
			{
				l[i].arr[j]=arr[k];
				k++;
			}
		}
		
		struct timeval start, end;
		gettimeofday( &start, NULL );
		
		for(int i=0;i<3;i++)
		{
			MPI_Send(l[i].arr,l[i].s,MPI_INT,i+1,0,MPI_COMM_WORLD);
			MPI_Send(&choice,1,MPI_INT,i+1,3,MPI_COMM_WORLD);
		}
		
		k=0;
		int op=0;
		for(int i=0;i<3;i++)
		{
			int* buf = (int*)malloc(sizeof(int) * size/3);
			MPI_Recv(buf,size/3,MPI_INT,i+1,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			MPI_Recv(&op,1,MPI_INT,i+1,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			operation_count+=op;
			for(int i=0;i<size/3;i++)
			{
				arr[k]=buf[i];
				k++;
			}
		}
		mergeSort(arr,0,size-1);
		
		if(choice==1)
			mergeSort(arr,0,size-1);
		else
			quickSort(arr,0,size-1);
			
		gettimeofday( &end, NULL );
		
		double seconds = (end.tv_sec - start.tv_sec) +1.0e-6 * (end.tv_usec - start.tv_usec);
	    	double Gflops = 2e-9*operation_count/seconds;
		
		if(choice==1)
			out<<"Merge Sort of size "<<size<<":"<<endl;
		else
			out<<"Quick Sort of size "<<size<<":"<<endl;
		for(int i=0;i<size;i++)
			out<<arr[i]<<" ";
			
		if(choice==1)
			t<<"Merge Sort of size "<<size<<"\t"<<Gflops<<endl<<endl;
		else
			t<<"Quick Sort of size "<<size<<"\t"<<Gflops<<endl<<endl;
			
		cout<<"Initial unsorted array is stored in inital.txt. Sorted Array is stored in output.txt"<<endl;
		cout<<"GFlops is stored in time.txt"<<endl;
	}
	else
	{ 
                
		int number_amount;
		int choice;
		MPI_Status status;
		MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
		MPI_Get_count(&status, MPI_INT, &number_amount);
		int* buf = (int*)malloc(sizeof(int) * number_amount);
		MPI_Recv(buf, number_amount, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&choice, 1, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		if(choice==1)
			mergeSort(buf,0,number_amount-1);
		else
			quickSort(buf,0,number_amount-1);
		MPI_Send(buf,number_amount,MPI_INT,0,1,MPI_COMM_WORLD);
		MPI_Send(&operation_count,1,MPI_INT,0,2,MPI_COMM_WORLD);
		
	}
	MPI_Finalize();
	out.close();
	t.close(); 
	initial.close();
	return 0;
}
