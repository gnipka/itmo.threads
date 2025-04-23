kernel void add(global const int *a, global const int *b, global int *c)
{
	int i = get_global_id(0);
	c[i] = a[i] + b[i];
}