#ifndef IALGORITHM_H
#define IALGORITHM_H

enum ALGORITHM_TYPE
{
	ALGORITHM_KDTREE,
	ALGORITHM_HARRIS3D,
	ALGORITHM_ALPHASHAPE,
	ALGORITHM_DELAUNAY
};
class IAlgorithm
{
public:
	IAlgorithm() {};
	~IAlgorithm() {};
	virtual ALGORITHM_TYPE GetType() = 0;
	virtual void Execute() = 0;
	virtual void ShowUI() = 0;
private:

};

#endif