#define _CRT_SECURE_NO_WARNINGS
#include "objReader.h"

void obj::ReadObj()
{
	//--- 1. ��ü ���ؽ� ���� �� �ﰢ�� ���� ����
	char count[100];

	while (!feof(objFile)) {
		fscanf(objFile, "%s", count);
		if (count[0] == 'v' && count[1] == '\0')
			vertexNum += 1;
		else if (count[0] == 'f' && count[1] == '\0')
			faceNum += 1;
		memset(count, '\0', sizeof(count)); // �迭 �ʱ�ȭ
	}
	//--- 2. �޸� �Ҵ�

	int vertIndex = 0;
	int faceIndex = 0;
	if (vertex != NULL) free(vertex);
	if (face_v != NULL) free(face_v);

	vertex = (glm::vec3*)malloc(sizeof(glm::vec3) * vertexNum);
	face_v = (int*)malloc(sizeof(int) * faceNum * 3);
	face_vt = (int*)malloc(sizeof(int) * faceNum * 3);
	face_vn = (int*)malloc(sizeof(int) * faceNum * 3);

	rewind(objFile);

	//--- 3. �Ҵ�� �޸𸮿� �� ���ؽ�, ���̽� ���� �Է�
	while (!feof(objFile)) {
		fscanf(objFile, "%s", count);
		if (count[0] == 'v' && count[1] == '\0') {
			fscanf(objFile, "%f %f %f",
				&vertex[vertIndex].x, &vertex[vertIndex].y,
				&vertex[vertIndex].z);
			vertIndex++;
		}
		else if (count[0] == 'f' && count[1] == '\0') {
			fscanf(objFile, "%d//%d %d//%d %d//%d",
				&face_v[faceIndex * 3], &face_vn[faceIndex * 3],
				&face_v[faceIndex * 3 + 1], &face_vn[faceIndex * 3 + 1],
				&face_v[faceIndex * 3 + 2], &face_vn[faceIndex * 3 + 2]);

			face_v[faceIndex * 3]--; face_v[faceIndex * 3 + 1]--; face_v[faceIndex * 3 + 2]--;
			face_vn[faceIndex * 3]--; face_vn[faceIndex * 3 + 1]--; face_vn[faceIndex * 3 + 2]--;
			faceIndex++;
		}
	}
}