kernel void multiply(global const float *a, global const float *b, global float *c, uint M, uint N, uint K)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	
	if (x >= M || y >= N) return;
	float sum = 0.0f;
	
	for (int i = 0; i < K; i++) {
        sum += a[x * K + i] * b[i * N + y];
    }

    c[x * N + y] = sum;
}