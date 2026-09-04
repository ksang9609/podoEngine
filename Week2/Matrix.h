#pragma once

struct FMatrix { 
	float M[4][4];
	float x, y, z, w;
	static const FMatrix Identity;

	FMatrix(float _x = 0, float _y = 0, float _z=0,float _w) : x(_x), y(_y), z(_z), w(_w) {}
	FMatrix() {

	}
	//역행렬
	//정규직교행렬
	//직교행렬

	FMatrix ReverseMatrix(FMatrix) {
		
	}

	static FMatrix dot(const FMatrix& A, const FMatrix& B) { //4 x 4
		//FMatrix result= FMatrix();

		for (int i = 0;i < 4;i++) {
			for (int j = 0;j < 4;j++) {
				//result[i][j]=
			}
		}
	}

};
struct FMatrix44f {
	float x, y, z, w;
};

struct FMatrix44d {

};
//struct FMatrixOrthonormal { // 정규직교행렬
//
//};
//
//struct FMatrixOrthogonal { // 직교행렬
//
//};
//
//struct FMatrixInverse { //역행렬
//
//};
