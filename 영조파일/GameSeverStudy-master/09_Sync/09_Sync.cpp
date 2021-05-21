#include <iostream>
#include <thread>
using namespace std;

// �޸� �ϰ��� �׽�Ʈ
// �޸��� ������ �Ѽ����� ������Ʈ ���� ���� ���� �ִ�.
// ���ؽ��� �ɸ� �ذ�ȴ�.

volatile int* bounce;
volatile bool done = false;
int error = 0;

void changer() {
	for (int i = 0; i < 25000000; i++) {
		*bounce = -(1 + *bounce);
	}
	done = true;
}

void checker() {
	while(false == done){
		int v = *bounce;
		if ((v != 0) && (v != -1)){
			error++;
			printf("%X, ", v); // 2����Ʈ �и��� 0000FFFF�� FFFF0000���� �ٲ��.
			// v�� �̻��� ���� �Ǵ� ������ ������ؼ� ��������.
		}
	}
}

void main() {
	for (int i = 0; i < 1'000'000; ++i) {
		int arr[17];
		auto addr = reinterpret_cast<long long>(&(arr[16])); // ĳ�ö����� 64����Ʈ�� �̷��� �ϴ°Ű���
		int offset = addr % 64;
		addr = addr - offset;
		addr = addr - 2; //�� ������ ������ �ַ��� 0�̾��µ� ������ �߰��ѰŸ����� �ַ��� ����.
		bounce = reinterpret_cast<int*>(addr);
		*bounce = 0;
		thread t1{ changer};
		thread t2{ checker};
		t1.join();
		t2.join();
		cout << "error: " << error << endl;
		done = false;
		error = 0;
	}
	// �����͸� ũ�� �о ĳ�ÿ� �±׸� ���δ�. ���� �������� ũ�Ⱑ ĳ�ú��� Ŀ���� �ȴ�. (�����Ҹ���)
	// ĳ���� ������ 64�� ����� �ڸ���.
	// ĳ�ö����� 64����Ʈ�� ������ ��ġ��ŷ�� ������ ���� ������ ȿ�����̾��
	// �ٿ���� ������ ����Ʈ�� �Ⱦ��� �˾Ƽ� �ذ��
	// ���� cpu�� ���� 32��Ʈ cpu�� �޸� ������ 32��Ʈ�ε� �ּҰ� ������ 4�� ������� �ѹ��� �������ִ�. 4�� ����� �ƴϸ� 2���� ���ļ� �о���Ѵ�. int a�� �����ּҰ� 102�ּ�ó�� 2�� ����̸� �׾������.
}