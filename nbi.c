#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define SWAP(a,b) { int t;t=a;a=b;b=t; }

int number_recommend;

typedef struct net
{
	int node;
	int degree;
	int* neighbor;
	double value;
}bipatite;

int* append(int *arr, int size, int value);
int* list_del(int *ar,int size, int idx);
int BinarySearch_raw(int *ar,int num,int key);
int BinarySearch(bipatite *ar,int num,int key);
void QuickSort(int *ar, int num);
void QuickSort_dual(int *ar1, double *ar2, int num);
int* ranking(bipatite *list, int size);
int unique(int *arr, int size);
void heat_diffusion(bipatite *main, bipatite *proj, int main_size, int proj_size);
void recommendation(bipatite *user, int *rank, int user_n, int rank_n);
void node_input(int *arr, int size, bipatite *node);
void network_making(int *link_left, int *link_right, int *purchase_date, bipatite *left, bipatite *right, int link_number, int left_node_number, int right_node_number);
bipatite* input_for_check(int *size_check, int direction);

int main(int argc, char **argv)
{
	FILE *indata;
	int i,j;
	bipatite *left; 
	bipatite *right; 
	int *link_left;
	int *link_right;
	int *purchase_date;
	int left_node_number, right_node_number;
	int cust, product, date, link_number=0;
	int *user_rank; 
	int *item_rank; 

	number_recommend=atoi(argv[1]);

	indata=fopen("training.txt", "r");
	while(!feof(indata))
	{
		fscanf(indata, "%d\t%d\t%d\n", &cust, &product, &date);
		link_number++;
	}
	rewind(indata);

	link_left=(int *)malloc(sizeof(int) * link_number);
	link_right=(int *)malloc(sizeof(int) * link_number);
	purchase_date=(int *)malloc(sizeof(int) * link_number);
	for(i=0; i<link_number; i++)
		fscanf(indata, "%d\t%d\t%d\n", &link_left[i], &link_right[i], &purchase_date[i]);

	left_node_number=unique(link_left, link_number);
	left=(bipatite *)malloc(sizeof(bipatite) *left_node_number);
	node_input(link_left, link_number, left);
	right_node_number=unique(link_right, link_number);
	right=(bipatite *)malloc(sizeof(bipatite) *right_node_number);
	node_input(link_right, link_number, right);

	network_making(link_left, link_right, purchase_date, left, right, link_number, left_node_number, right_node_number);

	/*
	   heat_diffusion(left, right, left_node_number, right_node_number);
	   user_rank=ranking(left, left_node_number);

	   FILE *user_recommend;
	   user_recommend=fopen("user_result.txt", "w");
	   for(i=0; i<left_node_number; i++)
	   fprintf(user_recommend, "%d\n", user_rank[i]);
	   fclose(user_recommend);
	   free(user_rank);
	 */

	heat_diffusion(right, left, right_node_number, left_node_number);
	item_rank=ranking(right, right_node_number);

	recommendation(left, item_rank, left_node_number, right_node_number);

	fclose(indata);

	for(i=0; i<left_node_number; i++)
		free(left[i].neighbor);

	for(i=0; i<right_node_number; i++)
		free(right[i].neighbor);

	free(item_rank);
	free(left);
	free(right);
	free(link_left);
	free(link_right);
	free(purchase_date);

	return 0;
}

int unique(int *arr, int size)
{
	int i;
	int *temp_arr;
	int temp;
	int number;
	number=1;

	temp_arr=(int *)malloc(sizeof(int) * size);

	for(i=0; i<size; i++)
		temp_arr[i]=arr[i];

	QuickSort(temp_arr, size);

	temp=temp_arr[0];

	for(i=0; i<size; i++)
	{
		if(temp == temp_arr[i])
			continue;
		else
		{
			number++;
			temp=temp_arr[i];
		}
	}

	free(temp_arr);
	return number;
}

void node_input(int *arr, int size, bipatite *node)
{
	int i, j;
	int temp;
	int *temp_arr;

	temp_arr=(int *)malloc(sizeof(int) * size);

	for(i=0; i<size; i++)
		temp_arr[i]=arr[i];

	QuickSort(temp_arr, size);

	temp=temp_arr[0];
	node[0].node=temp_arr[0];
	j=1;

	for(i=0; i<size; i++)
	{
		if(temp == temp_arr[i])
			continue;
		else
		{
			node[j].node=temp_arr[i];
			temp=temp_arr[i];
			j++;
		}
	}

	free(temp_arr);
}

void network_making(int *link_left, int *link_right, int *purchase_date, bipatite *left, bipatite *right, int link_number, int left_node_number, int right_node_number)
{
	int i;
	int net_index;
	for(i=0; i<left_node_number; i++)
		left[i].degree=0;
	for(i=0; i<right_node_number; i++)
		right[i].degree=0;

	for(i=0; i<link_number; i++)
	{
		net_index=BinarySearch(left, left_node_number, link_left[i]);
		left[net_index].neighbor=append(left[net_index].neighbor, left[net_index].degree, link_right[i]);
		left[net_index].degree++;

		net_index=BinarySearch(right, right_node_number, link_right[i]);
		right[net_index].neighbor=append(right[net_index].neighbor, right[net_index].degree, link_left[i]);
		right[net_index].degree++;
	}
}

void QuickSort(int *ar, int num)
{
	int left,right;
	int key;

	// 구간이 1이면 정렬 끝
	if (num <= 1) return;

	// 기준값 결정 : 배열상의 제일 끝 요소
	key=ar[num-1];
	for (left=0,right=num-2;;left++,right--) {
		while (ar[left] < key) { left++; }
		while (ar[right] > key) { right--; }
		if (left >= right) break;            // 좌우가 만나면 끝
		SWAP(ar[left],ar[right]);
	}
	SWAP(ar[left],ar[num-1]);                   // 기준값과 i위치의 값 교환

	QuickSort(ar,left);                           // 왼쪽 구간 정렬
	QuickSort(ar+left+1,num-left-1);        // 오른쪽 구간 정렬
}

void QuickSort_dual(int *ar1, double *ar2, int num)
{
	int left,right;
	double key;

	// 구간이 1이면 정렬 끝
	if (num <= 1) return;

	// 기준값 결정 : 배열상의 제일 끝 요소
	key=ar2[num-1];
	for (left=0,right=num-2;;left++,right--) {
		while (ar2[left] < key) { left++; }
		while (ar2[right] > key) { right--; }
		if (left >= right) break;            // 좌우가 만나면 끝
		SWAP(ar1[left],ar1[right]);
		SWAP(ar2[left],ar2[right]);
	}
	SWAP(ar1[left],ar1[num-1]);                   // 기준값과 i위치의 값 교환
	SWAP(ar2[left],ar2[num-1]);                   // 기준값과 i위치의 값 교환

	QuickSort_dual(ar1, ar2,left);                           // 왼쪽 구간 정렬
	QuickSort_dual(ar1+left+1,ar2+left+1,num-left-1);        // 오른쪽 구간 정렬
}

int BinarySearch(bipatite *ar,int num,int key)
{
	int Upper,Lower,Mid;

	Lower=0;
	Upper=num-1;
	for (;;) {
		Mid=(Upper+Lower)/2;

		if (ar[Mid].node==key) return Mid;
		if (ar[Mid].node>key) {
			Upper=Mid-1;
		} else {
			Lower=Mid+1;
		}
		if (Upper<Lower) {
			return -1;
		}
	}
}

int* append(int *arr, int size, int value)
{
	int *temp;
	int i;
	int *edge;

	if(size == 0)
	{
		edge=(int *)malloc(sizeof(int));
		edge[0]=value;
	}

	else
	{
		temp=(int *)malloc(sizeof(int) * size);
		for(i=0; i<size; i++)
			temp[i]=arr[i];

		free(arr); //danger of memory loss
		edge=(int *)malloc(sizeof(int) * (size+1));
		for(i=0; i<size; i++)
			edge[i]=temp[i];

		free(temp);
		edge[size]=value;
	}

	return edge;
}

void heat_diffusion(bipatite *center, bipatite *proj, int center_size, int proj_size)
{
	int i, j;
	int index;

	for(i=0; i<center_size; i++)
		center[i].value=0;
	for(i=0; i<proj_size; i++)
		proj[i].value=0;

	for(i=0; i<center_size; i++)
		for(j=0; j<center[i].degree; j++)
		{
			index=BinarySearch(proj, proj_size, center[i].neighbor[j]);
			center[i].value+=(proj[index].degree-1);
		}

	int k;
	// one heat deffusion is started
	for(i=0; i<center_size; i++)
		for(j=0; j<center[i].degree; j++)
		{
			index=BinarySearch(proj, proj_size, center[i].neighbor[j]);
			proj[index].value+=(center[i].value/center[i].degree);
		}

	for(i=0; i<center_size; i++)
		center[i].value=0;

	for(i=0; i<proj_size; i++)
		for(j=0; j<proj[i].degree; j++)
		{
			index=BinarySearch(center, center_size, proj[i].neighbor[j]);
			center[index].value+=(proj[i].value/proj[i].degree);
		}

	for(i=0; i<proj_size; i++)
		proj[i].value=0;
	// one heat deffusion is edded 
}

void recommendation(bipatite *user, int *item_rank, int user_n, int rank_n)
{
	int i,j,k,l;
	int *recommend_table;
	int *idx;
	double *rank_temp;
	int *rank_seq;
	FILE *result;
	bipatite *check;
	int *size_check;
	int expect_collect=0; 
	int size;
	size_check=&size;

	check=input_for_check(size_check, 0);//check!

	result=fopen("result.txt", "w");

	idx=(int *)malloc(sizeof(int) * rank_n);
	rank_seq=(int *)malloc(sizeof(int) * rank_n);
	rank_temp=(double *)malloc(sizeof(double) * rank_n);
	for(i=0; i<rank_n; i++)
	{
		idx[i]=i;
		rank_temp[i]=(double)item_rank[i];
	}

	QuickSort_dual(idx, rank_temp, rank_n);

	for(i=0; i<rank_n; i++)
		rank_seq[i]=rank_temp[i];

	int length;
	for(i=0; i<user_n; i++)
	{
		length=number_recommend;
		recommend_table=(int *)malloc(sizeof(int) * rank_n);
		for(j=0; j<rank_n; j++)
			recommend_table[j]=item_rank[j];

		for(j=0; j<user[i].degree; j++)
		{
			k=BinarySearch_raw(rank_seq, rank_n, user[i].neighbor[j]);
			recommend_table[idx[k]]=0;
			length++;
		}

		//check start!!
		k=BinarySearch(check, size, user[i].node);
		if(k!=-1)
			for(j=0; j<check[k].degree; j++)
			{
				//	printf("%d\t%d\n", check[k].node, check[k].neighbor[j]);
				l=BinarySearch_raw(rank_seq, rank_n, check[k].neighbor[j]);
				if((l!=-1) && (idx[l] < length) && (recommend_table[idx[l]]!=0))
					expect_collect++;
			}
		//check ended!!

		/*
		   fprintf(result,"%d\t", user[i].node);
		   for(j=0; j<length; j++)
		   if(recommend_table[j] != 0)
		   fprintf(result,"%d\t", recommend_table[j]);
		   fprintf(result,"\n");
		 */

		free(recommend_table);
	}

	printf("%d\t%d\n", number_recommend, expect_collect);
	fclose(result);
	free(idx);
	free(rank_temp);
	free(rank_seq);
	for(i=0; i<size; i++)
		free(check[i].neighbor);

	free(check); //check! maybe lose of memory
}

int* ranking(bipatite *list, int size)
{
	int i;
	int *id;
	double *value;
	id=(int *)malloc(sizeof(int) * size);
	value=(double *)malloc(sizeof(double) * size);

	for(i=0; i<size; i++)
		id[i]=list[i].node;
	for(i=0; i<size; i++)
		value[i]=list[i].value;

	QuickSort_dual(id, value, size);

	free(value);
	return id;
}

int* list_del(int *ar,int size, int idx)
{
	int *x;
	int i;
	x=(int *)malloc(sizeof(int) * size);
	for(i=0; i<size; i++)
		x[i]=ar[i];

	free(ar); //danger of memory loss
	memmove(x+idx,x+idx+1, sizeof(int)*(size-idx));
	return x;
}

int BinarySearch_raw(int *ar,int num,int key)
{
	int Upper,Lower,Mid;

	Lower=0;
	Upper=num-1;
	for (;;) {
		Mid=(Upper+Lower)/2;

		if (ar[Mid]==key) return Mid;
		if (ar[Mid]>key) {
			Upper=Mid-1;
		} else {
			Lower=Mid+1;
		}
		if (Upper<Lower) {
			return -1;
		}
	}
}

bipatite* input_for_check(int *size_check, int direction)
{
	FILE *indata;
	int i,j;
	bipatite *left; 
	bipatite *right; 
	int *link_left;
	int *link_right;
	int *purchase_date;
	int left_node_number, right_node_number;
	int cust, product, date, link_number=0;

	indata=fopen("check.txt", "r");
	while(!feof(indata))
	{
		fscanf(indata, "%d\t%d\t%d\n", &cust, &product, &date);
		link_number++;
	}
	rewind(indata);

	link_left=(int *)malloc(sizeof(int) * link_number);
	link_right=(int *)malloc(sizeof(int) * link_number);
	purchase_date=(int *)malloc(sizeof(int) * link_number);
	for(i=0; i<link_number; i++)
		fscanf(indata, "%d\t%d\t%d\n", &link_left[i], &link_right[i], &purchase_date[i]);

	left_node_number=unique(link_left, link_number);
	left=(bipatite *)malloc(sizeof(bipatite) *left_node_number);
	node_input(link_left, link_number, left);
	right_node_number=unique(link_right, link_number);
	right=(bipatite *)malloc(sizeof(bipatite) *right_node_number);
	node_input(link_right, link_number, right);

	network_making(link_left, link_right, purchase_date, left, right, link_number, left_node_number, right_node_number);

	free(link_left);
	free(link_right);
	free(purchase_date);
	fclose(indata);
	*size_check=left_node_number;

	if(direction == 0)
	{
		for(i=0; i<right_node_number; i++)
			free(right[i].neighbor);
		free(right);
		return left;
	}
	else
	{
		for(i=0; i<left_node_number; i++)
			free(left[i].neighbor);
		free(left);
		return right;
	}

}

