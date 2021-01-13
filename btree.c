#include "btree.h"

#include <stdlib.h>
#include <stdio.h>

btree_t* btree_init(int value, void* data)
{
	btree_t *root = (btree_t*)malloc(sizeof(btree_t));
	root->value = value;
	root->left = NULL;
	root->right = NULL;
	return root;
}

void btree_destroy(btree_t *root)
{
	if (root->left)
	{
		btree_destroy(root->left);
	}
	if (root->right)
	{
		btree_destroy(root->right);
	}
	free(root);
}

void btree_add(btree_t *root, int value, void* data)
{
	if (value < root->value)
	{
		if (root->left)
		{
			btree_add(root->left, value, data);
		}
		else
		{
			root->left = btree_init(value, data);
		}
	}
	else
	{
		if (root->right)
		{
			btree_add(root->right, value, data);
		}
		else
		{
			root->right = btree_init(value, data);
		}
	}
}

void* btree_find(btree_t *root, int value)
{
	printf("passing %i\n", root->value);
	if (root->value == value)
	{
		return root->data;
	}
	else if (value < root->value && root->left)
	{
		return btree_find(root->left, value);
	}
	else if (root->right)
	{
		return btree_find(root->right, value);
	}
	return NULL;
}
