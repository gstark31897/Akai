struct btree_t;
typedef struct btree_t {
	int value;
	void* data;
	struct btree_t *left, *right;
} btree_t;

btree_t* btree_init(int value, void* data);
void btree_destroy(btree_t* root);
void btree_add(btree_t* root, int value, void* data);
void* btree_find(btree_t* root, int value);
