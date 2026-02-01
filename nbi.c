#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define SWAP(a,b) { int t;t=a;a=b;b=t; }
#define SWAP_DOUBLE(a,b) { double t;t=a;a=b;b=t; }

int number_recommend;

typedef struct net
{
	int node;
	int degree;
	int* neighbor;
	double value;
}bipatite;

int* append(int *arr, int size, int value);
int BinarySearch_raw(int *ar,int num,int key);
int BinarySearch(bipatite *ar,int num,int key);
void QuickSort(int *ar, int num);
void QuickSort_dual_desc(int *ar1, double *ar2, int num);
int unique(int *arr, int size);
void node_input(int *arr, int size, bipatite *node);
void network_making(int *link_left, int *link_right, bipatite *left, bipatite *right, int link_number, int left_node_number, int right_node_number);
bipatite* input_for_check(int *size_check);
void nbi_recommendation(bipatite *users, bipatite *items, int user_n, int item_n, bipatite *check, int check_size);

int main(int argc, char **argv)
{
	FILE *indata;
	int i;
	bipatite *users;
	bipatite *items;
	int *link_left;
	int *link_right;
	int user_count, item_count;
	int cust, product, date, link_number=0;
	bipatite *check;
	int check_size;

	if(argc < 2)
	{
		printf("Usage: %s <number_of_recommendations>\n", argv[0]);
		return 1;
	}
	number_recommend = atoi(argv[1]);

	indata = fopen("training.txt", "r");
	if(!indata)
	{
		fprintf(stderr, "Error: Cannot open training.txt\n");
		return 1;
	}

	while(fscanf(indata, "%d\t%d\t%d\n", &cust, &product, &date) == 3)
		link_number++;
	rewind(indata);

	if(link_number == 0)
	{
		fprintf(stderr, "Error: No data in training.txt\n");
		fclose(indata);
		return 1;
	}

	link_left = (int *)malloc(sizeof(int) * link_number);
	link_right = (int *)malloc(sizeof(int) * link_number);
	if(!link_left || !link_right)
	{
		fprintf(stderr, "Error: Memory allocation failed\n");
		fclose(indata);
		free(link_left);
		free(link_right);
		return 1;
	}

	for(i=0; i<link_number; i++)
		fscanf(indata, "%d\t%d\t%d\n", &link_left[i], &link_right[i], &date);
	fclose(indata);

	user_count = unique(link_left, link_number);
	users = (bipatite *)malloc(sizeof(bipatite) * user_count);
	if(!users)
	{
		fprintf(stderr, "Error: Memory allocation failed for users\n");
		free(link_left);
		free(link_right);
		return 1;
	}
	node_input(link_left, link_number, users);

	item_count = unique(link_right, link_number);
	items = (bipatite *)malloc(sizeof(bipatite) * item_count);
	if(!items)
	{
		fprintf(stderr, "Error: Memory allocation failed for items\n");
		free(users);
		free(link_left);
		free(link_right);
		return 1;
	}
	node_input(link_right, link_number, items);

	network_making(link_left, link_right, users, items, link_number, user_count, item_count);

	check = input_for_check(&check_size);
	if(!check)
	{
		fprintf(stderr, "Warning: Could not load check.txt, skipping evaluation\n");
		check_size = 0;
	}

	nbi_recommendation(users, items, user_count, item_count, check, check_size);

	for(i=0; i<user_count; i++)
		free(users[i].neighbor);
	for(i=0; i<item_count; i++)
		free(items[i].neighbor);
	for(i=0; i<check_size; i++)
		free(check[i].neighbor);

	free(users);
	free(items);
	free(check);
	free(link_left);
	free(link_right);

	return 0;
}

/*
 * NBI (Network-Based Inference) Algorithm
 * Based on: Zhou et al., "Bipartite network projection and personal recommendation", Phys. Rev. E 76, 046115 (2007)
 *
 * For each user i:
 *   Step 1: Assign initial resource f=1 to items that user i has selected
 *   Step 2: Diffuse resources from Items -> Users: f(u) = sum(f(item) / k(item))
 *   Step 3: Diffuse resources from Users -> Items: f'(item) = sum(f(u) / k(u))
 *   Step 4: Rank items by f' score, excluding already selected items
 */
void nbi_recommendation(bipatite *users, bipatite *items, int user_n, int item_n, bipatite *check, int check_size)
{
	int i, j, k, l;
	int index;
	double *item_resource;
	double *user_resource;
	double *final_score;
	int *item_ids;
	FILE *result;
	int total_hits = 0;

	if(user_n == 0 || item_n == 0)
	{
		fprintf(stderr, "Error: No users or items to process\n");
		return;
	}

	item_resource = (double *)malloc(sizeof(double) * item_n);
	user_resource = (double *)malloc(sizeof(double) * user_n);
	final_score = (double *)malloc(sizeof(double) * item_n);
	item_ids = (int *)malloc(sizeof(int) * item_n);

	if(!item_resource || !user_resource || !final_score || !item_ids)
	{
		fprintf(stderr, "Error: Memory allocation failed in nbi_recommendation\n");
		free(item_resource);
		free(user_resource);
		free(final_score);
		free(item_ids);
		return;
	}

	result = fopen("result.txt", "w");
	if(!result)
	{
		perror("Error opening result.txt");
		free(item_resource);
		free(user_resource);
		free(final_score);
		free(item_ids);
		return;
	}

	/* Initialize item_ids once before the loop (optimization) */
	for(j=0; j<item_n; j++)
		item_ids[j] = items[j].node;

	for(i=0; i<user_n; i++)
	{
		/* Initialize resources to zero */
		for(j=0; j<item_n; j++)
		{
			item_resource[j] = 0.0;
			final_score[j] = 0.0;
		}
		for(j=0; j<user_n; j++)
			user_resource[j] = 0.0;

		/* Restore item_ids order for this iteration */
		for(j=0; j<item_n; j++)
			item_ids[j] = items[j].node;

		/* Step 1: Assign initial resource = 1 to items selected by user i */
		for(j=0; j<users[i].degree; j++)
		{
			index = BinarySearch(items, item_n, users[i].neighbor[j]);
			if(index != -1)
				item_resource[index] = 1.0;
		}

		/* Step 2: Items -> Users diffusion
		 * f(user_l) = sum over connected items: f(item) / k(item)
		 */
		for(j=0; j<item_n; j++)
		{
			if(item_resource[j] > 0)
			{
				for(k=0; k<items[j].degree; k++)
				{
					index = BinarySearch(users, user_n, items[j].neighbor[k]);
					if(index != -1)
						user_resource[index] += item_resource[j] / items[j].degree;
				}
			}
		}

		/* Step 3: Users -> Items diffusion
		 * f'(item_j) = sum over connected users: f(user) / k(user)
		 */
		for(j=0; j<user_n; j++)
		{
			if(user_resource[j] > 0)
			{
				for(k=0; k<users[j].degree; k++)
				{
					index = BinarySearch(items, item_n, users[j].neighbor[k]);
					if(index != -1)
						final_score[index] += user_resource[j] / users[j].degree;
				}
			}
		}

		/* Step 4: Set score=0 for items already selected by user (exclude from recommendation) */
		for(j=0; j<users[i].degree; j++)
		{
			index = BinarySearch(items, item_n, users[i].neighbor[j]);
			if(index != -1)
				final_score[index] = 0.0;
		}

		/* Sort items by final_score in descending order */
		QuickSort_dual_desc(item_ids, final_score, item_n);

		/* Evaluate: check how many items in test set are in top-N recommendations */
		if(check && check_size > 0)
		{
			int check_idx = BinarySearch(check, check_size, users[i].node);
			if(check_idx != -1)
			{
				for(j=0; j<number_recommend && j<item_n; j++)
				{
					for(l=0; l<check[check_idx].degree; l++)
					{
						if(item_ids[j] == check[check_idx].neighbor[l])
						{
							total_hits++;
							break;
						}
					}
				}
			}
		}

		/* Output recommendations for this user */
		fprintf(result, "%d", users[i].node);
		for(j=0; j<number_recommend && j<item_n; j++)
		{
			if(final_score[j] > 0)
				fprintf(result, "\t%d", item_ids[j]);
		}
		fprintf(result, "\n");
	}

	printf("Recommendations: %d, Hits: %d, Precision: %.4f\n",
		   number_recommend, total_hits,
		   (user_n > 0 ? (double)total_hits / (number_recommend * user_n) : 0.0));

	fclose(result);
	free(item_resource);
	free(user_resource);
	free(final_score);
	free(item_ids);
}

int unique(int *arr, int size)
{
	int i;
	int *temp_arr;
	int temp;
	int number = 1;

	if(size == 0)
		return 0;

	temp_arr = (int *)malloc(sizeof(int) * size);
	if(!temp_arr)
	{
		fprintf(stderr, "Error: Memory allocation failed in unique\n");
		exit(EXIT_FAILURE);
	}

	for(i=0; i<size; i++)
		temp_arr[i] = arr[i];

	QuickSort(temp_arr, size);
	temp = temp_arr[0];

	for(i=1; i<size; i++)
	{
		if(temp != temp_arr[i])
		{
			number++;
			temp = temp_arr[i];
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

	if(size == 0)
		return;

	temp_arr = (int *)malloc(sizeof(int) * size);
	if(!temp_arr)
	{
		fprintf(stderr, "Error: Memory allocation failed in node_input\n");
		exit(EXIT_FAILURE);
	}

	for(i=0; i<size; i++)
		temp_arr[i] = arr[i];

	QuickSort(temp_arr, size);

	temp = temp_arr[0];
	node[0].node = temp_arr[0];
	j = 1;

	for(i=1; i<size; i++)
	{
		if(temp != temp_arr[i])
		{
			node[j].node = temp_arr[i];
			temp = temp_arr[i];
			j++;
		}
	}

	free(temp_arr);
}

void network_making(int *link_left, int *link_right, bipatite *left, bipatite *right, int link_number, int left_node_number, int right_node_number)
{
	int i;
	int net_index;

	for(i=0; i<left_node_number; i++)
	{
		left[i].degree = 0;
		left[i].neighbor = NULL;
	}
	for(i=0; i<right_node_number; i++)
	{
		right[i].degree = 0;
		right[i].neighbor = NULL;
	}

	for(i=0; i<link_number; i++)
	{
		net_index = BinarySearch(left, left_node_number, link_left[i]);
		left[net_index].neighbor = append(left[net_index].neighbor, left[net_index].degree, link_right[i]);
		left[net_index].degree++;

		net_index = BinarySearch(right, right_node_number, link_right[i]);
		right[net_index].neighbor = append(right[net_index].neighbor, right[net_index].degree, link_left[i]);
		right[net_index].degree++;
	}
}

void QuickSort(int *ar, int num)
{
	int left, right;
	int key;

	if(num <= 1) return;

	key = ar[num-1];
	for(left=0, right=num-2; ; left++, right--)
	{
		while(ar[left] < key) left++;
		while(right >= 0 && ar[right] > key) right--;
		if(left >= right) break;
		SWAP(ar[left], ar[right]);
	}
	SWAP(ar[left], ar[num-1]);

	QuickSort(ar, left);
	QuickSort(ar+left+1, num-left-1);
}

/* QuickSort in DESCENDING order by ar2 values */
void QuickSort_dual_desc(int *ar1, double *ar2, int num)
{
	int left, right;
	double key;

	if(num <= 1) return;

	key = ar2[num-1];
	for(left=0, right=num-2; ; left++, right--)
	{
		while(ar2[left] > key) left++;  /* descending: > instead of < */
		while(right >= 0 && ar2[right] < key) right--;  /* descending: < instead of > */
		if(left >= right) break;
		SWAP(ar1[left], ar1[right]);
		SWAP_DOUBLE(ar2[left], ar2[right]);
	}
	SWAP(ar1[left], ar1[num-1]);
	SWAP_DOUBLE(ar2[left], ar2[num-1]);

	QuickSort_dual_desc(ar1, ar2, left);
	QuickSort_dual_desc(ar1+left+1, ar2+left+1, num-left-1);
}

int BinarySearch(bipatite *ar, int num, int key)
{
	int upper, lower, mid;

	lower = 0;
	upper = num - 1;
	while(lower <= upper)
	{
		mid = (upper + lower) / 2;
		if(ar[mid].node == key) return mid;
		if(ar[mid].node > key)
			upper = mid - 1;
		else
			lower = mid + 1;
	}
	return -1;
}

int BinarySearch_raw(int *ar, int num, int key)
{
	int upper, lower, mid;

	lower = 0;
	upper = num - 1;
	while(lower <= upper)
	{
		mid = (upper + lower) / 2;
		if(ar[mid] == key) return mid;
		if(ar[mid] > key)
			upper = mid - 1;
		else
			lower = mid + 1;
	}
	return -1;
}

int* append(int *arr, int size, int value)
{
	int *new_arr = realloc(arr, sizeof(int) * (size + 1));
	if(!new_arr)
	{
		perror("realloc failed in append");
		exit(EXIT_FAILURE);
	}
	new_arr[size] = value;
	return new_arr;
}

bipatite* input_for_check(int *size_check)
{
	FILE *indata;
	int i;
	bipatite *users;
	bipatite *items;
	int *link_left;
	int *link_right;
	int user_count, item_count;
	int cust, product, date, link_number = 0;

	indata = fopen("check.txt", "r");
	if(!indata)
	{
		fprintf(stderr, "Error: Cannot open check.txt\n");
		*size_check = 0;
		return NULL;
	}

	while(fscanf(indata, "%d\t%d\t%d\n", &cust, &product, &date) == 3)
		link_number++;
	rewind(indata);

	if(link_number == 0)
	{
		fprintf(stderr, "Error: No data in check.txt\n");
		fclose(indata);
		*size_check = 0;
		return NULL;
	}

	link_left = (int *)malloc(sizeof(int) * link_number);
	link_right = (int *)malloc(sizeof(int) * link_number);
	if(!link_left || !link_right)
	{
		fprintf(stderr, "Error: Memory allocation failed in input_for_check\n");
		fclose(indata);
		free(link_left);
		free(link_right);
		*size_check = 0;
		return NULL;
	}

	for(i=0; i<link_number; i++)
		fscanf(indata, "%d\t%d\t%d\n", &link_left[i], &link_right[i], &date);
	fclose(indata);

	user_count = unique(link_left, link_number);
	users = (bipatite *)malloc(sizeof(bipatite) * user_count);
	if(!users)
	{
		fprintf(stderr, "Error: Memory allocation failed for check users\n");
		free(link_left);
		free(link_right);
		*size_check = 0;
		return NULL;
	}
	node_input(link_left, link_number, users);

	item_count = unique(link_right, link_number);
	items = (bipatite *)malloc(sizeof(bipatite) * item_count);
	if(!items)
	{
		fprintf(stderr, "Error: Memory allocation failed for check items\n");
		free(users);
		free(link_left);
		free(link_right);
		*size_check = 0;
		return NULL;
	}
	node_input(link_right, link_number, items);

	network_making(link_left, link_right, users, items, link_number, user_count, item_count);

	free(link_left);
	free(link_right);

	for(i=0; i<item_count; i++)
		free(items[i].neighbor);
	free(items);

	*size_check = user_count;
	return users;
}
