#ifndef BINPACKER2D_H
#define BINPACKER2D_H

#include <vector>
#include <algorithm>

namespace bp2D
{

#define BINPACKPARAM_POWEROFTWO 0x1
#define BINPACKPARAM_ASS

enum SORT_METHOD
{
	SORT_MAXSIDE,
	SORT_SQUARE,
	SORT_WIDTH,
	SORT_HEIGHT
};

struct BinRect
{
	unsigned int id;
	float x, y;
	float w, h;

	BinRect()
	{
		this->id = 0;
		this->x = 0;
		this->y = 0;
		this->w = 0;
		this->h = 0;
	}

	BinRect(int id, float w, float h)
	{
		this->id = id;
		this->x = 0;
		this->y = 0;
		this->w = w;
		this->h = h;
	}

	BinRect(int id, float x, float y, float w, float h)
	{
		this->id = id;
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}

	BinRect(float w, float h)
	{
		this->id = 0;
		this->x = 0;
		this->y = 0;
		this->w = w;
		this->h = h;
	}

	BinRect(float x, float y, float w, float h)
	{
		this->id = 0;
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}

	float GetMaxSide()
	{
		if(w>h)
			return w;
		else
			return h;
	}

	float GetSquare()
	{
		return (w*h);
	}
/*
	bool operator > (BinRect br)
	{return this->GetMaxSide() > br.GetMaxSide();}

	bool operator < (BinRect br)
	{return this->GetMaxSide() < br.GetMaxSide();}

	bool operator == (BinRect br)
	{return this->GetMaxSide() == br.GetMaxSide();}*/
};

class BinNode
{
public:
	BinNode()
	{
		children[0] = 0;
		children[1] = 0;
		isFilled = false;
		parent = 0;
	}
	BinNode(BinNode* nodeToCopy)
	{
		children[0] = nodeToCopy->GetChildNode(0);
		children[1] = nodeToCopy->GetChildNode(1);
		isFilled = nodeToCopy->IsFilled();
		parent = nodeToCopy->GetParentNode();
		SetRect(nodeToCopy->GetRect());
	}
	~BinNode(){}

	BinNode* Insert(BinRect bin)
	{
		//Recursively find fitting leaf node for our bin
		BinNode* bn = this->_findEmptyLeafNode(bin);

		if(bn == 0)
		{
			_expand(bin);
			bn = this->_findEmptyLeafNode(bin);
		}

		if(bn)
		{
			//!!!Found fitting leaf node. Putting it in.
			bn->Fit(bin);
			
		}
		else
		{
			//Couldn't fit even after _expand(). There's something wrong with your code.
		}

		return bn;

		/*
		if(!IsLeaf())
		{
			//Not a leaf. Try to pass bin to children.
			if(children[0]->Insert(bin)==0)
				if(children[1]->Insert(bin)==0)
				{
					//Block doesn't fit in both child nodes
					if(!_expand(bin)) //Try to expand, this will fail if we are not the root node
					{
						return 0;
					}
				}
		}

		if(IsFilled())
		{
			if(parent==0)
			{
				_expand(bin);
			}
			else
				return 0;
		}
		//This node is a leaf and is empty

		return Fit(bin);*/
	}

	BinNode* Fit(BinRect bin)
	{	
		if(bin.w == rect.w && bin.h == rect.h)
		{
			//Fits perfectly
			this->rect.id = bin.id;
			this->isFilled = true;
			return this;
		}
		else if(rect.w >= bin.w && rect.h >= bin.h)
		{
			//Volume is larger than passed rect
			if((rect.w-bin.w) > (rect.h-bin.h))
			{
				//_split(true, bin.h);
				_split(false, bin.w);
			}
			else
			{
				//_split(false, bin.w);
				_split(true, bin.h);
			}

			BinNode* bn = 0;
			if((bn = children[0]->Fit(bin))==0)
				return children[1]->Fit(bin);

			return bn;
		}
		else
		{
			//Doesn't fit.
			return 0;
		}
	}

	BinNode* _findEmptyLeafNode(BinRect bin)
	{
		if(!IsLeaf())
		{
			//Not a leaf.
			BinNode* bn = 0;
			if((bn = children[0]->_findEmptyLeafNode(bin))==0)
				if((bn = children[1]->_findEmptyLeafNode(bin))==0)
					return 0;
			return bn;
		}

		if(IsFilled())
		{
			return 0;
		}
		
		if(rect.w>=bin.w && rect.h>=bin.h)
		{
			return this;
		}
		else
			return 0;
	}

	bool IsLeaf()
	{	return ((children[0] == 0) && (children[1] == 0));	}

	bool IsFilled()
	{
		if(IsLeaf())
			return isFilled;
		else
			return false;
	}

	BinNode* GetChildNode(int i)
	{	return children[i];	}
	BinNode* GetParentNode()
	{	return parent;	}
	BinRect GetRect()
	{	return rect;	}

	void FillVolumeVector(std::vector<BinRect> &vec)
	{
		if(IsLeaf())
		{
			if(IsFilled())
			{
				vec.insert(vec.end(), rect);
				return;
			}
			return;
		}
		else
		{
			children[0]->FillVolumeVector(vec);
			children[1]->FillVolumeVector(vec);
		}
	}

	void SetSize(float w, float h)
	{
		this->rect.w = w;
		this->rect.h = h;
	}

	void SetPos(float x, float y)
	{
		this->rect.x = x;
		this->rect.y = y;
	}

	void SetRect(BinRect r)
	{
		rect = r;
	}

	void SetParent(BinNode* p)
	{
		parent = p;
	}
private:
	BinNode* parent;
	BinNode* children[2];
	BinRect rect;
	bool isFilled;

	

	//Tries to split this node into two. But if passed bin fits perfectly, just returns this node. Otherwise returns first child node.
	void _split(bool horizontalSplit, float offset)
	{
		if(children[0]!=0 || children[1]!=0)
			return;

		children[0] = new BinNode();
		children[0]->SetParent(this);
		children[1] = new BinNode();
		children[1]->SetParent(this);

		if(horizontalSplit)
		{
			children[0]->SetRect(BinRect(rect.x, rect.y, rect.w, offset));
			children[1]->SetRect(BinRect(rect.x, rect.y+offset, rect.w, rect.h-offset));
			rect.h = rect.h + offset;
		}
		else
		{
			children[0]->SetRect(BinRect(rect.x, rect.y, offset, rect.h));
			children[1]->SetRect(BinRect(rect.x+offset, rect.y, rect.w-offset, rect.h));
			rect.w = rect.w + offset;
		}

		//Node has been splitten.
	}	

	//Copy all node content to the first child, create second and fit rect inside
	bool _expand(BinRect bin)
	{
		if(parent == 0) //This is a root node
		{
			BinNode* newNode = new BinNode(this);
			children[0] = newNode;
			children[0]->SetParent(this);
			children[1] = new BinNode();
			children[1]->SetParent(this);
			this->isFilled = false;

			bool expandDown = rect.w > rect.h;
			if(expandDown)
			{
				if(bin.w > rect.w)
					expandDown = !expandDown;
			}
			else
			{
				if(bin.h > rect.h)
					expandDown = !expandDown;
			}

			if(expandDown)
			{
				children[1]->SetPos(0, rect.h);
				children[1]->SetSize(rect.w, bin.h);
				this->SetSize(rect.w, rect.h+bin.h);
				//Expanding down.
			}
			else
			{
				children[1]->SetPos(rect.w, 0);
				children[1]->SetSize(bin.w, rect.h);
				this->SetSize(rect.w+bin.w, rect.h);
				//Expanding right.
			}
			//Expand is done.
			return true;
		}
		else
			return false;
	}
};

inline bool BinRectCompareMaxSide(BinRect r0, BinRect r1)
{return r0.GetMaxSide()>r1.GetMaxSide();}
inline bool BinRectCompareWidth(BinRect r0, BinRect r1)
{return r0.w>r1.w;}
inline bool BinRectCompareHeight(BinRect r0, BinRect r1)
{return r0.h>r1.h;}
inline bool BinRectCompareSquare(BinRect r0, BinRect r1)
{return r0.GetSquare()>r1.GetSquare();}

class BinPacker2D
{
public:
	BinPacker2D()
	{
		rootNode = new BinNode();
	}
	~BinPacker2D()
	{
		volumes.clear();
		delete rootNode;
	}

	void AddRect(BinRect bin)
	{
		volumes.insert(volumes.end(), bin);
	}

	void Pack(int packingFlags = 0, SORT_METHOD sm = SORT_MAXSIDE)
	{
		if((packingFlags & BINPACKPARAM_POWEROFTWO) == BINPACKPARAM_POWEROFTWO){}
	
		switch(sm)
		{
		case SORT_MAXSIDE:
			//Sorting from largest to smallest rect (comparing by the largest side)
			std::sort(volumes.begin(), volumes.end(), BinRectCompareMaxSide);
			break;
		case SORT_WIDTH:
			std::sort(volumes.begin(), volumes.end(), BinRectCompareWidth);
			break;
		case SORT_HEIGHT:
			std::sort(volumes.begin(), volumes.end(), BinRectCompareHeight);
			break;
		case SORT_SQUARE:
			std::sort(volumes.begin(), volumes.end(), BinRectCompareSquare);
			break;
		default:
			std::sort(volumes.begin(), volumes.end(), BinRectCompareMaxSide);
			break;
		}
		//
		BinNode* n = 0;
		rootNode->SetSize(volumes[0].w, volumes[0].h);
		for(unsigned int i = 0; i < volumes.size(); i++)
		{
			n = rootNode->Insert(volumes[i]);
			volumes[i].x = n->GetRect().x;
			volumes[i].y = n->GetRect().y;
		}

		if(packingFlags & BINPACKPARAM_POWEROFTWO)
		{
			unsigned int pot_w = (unsigned)rootNode->GetRect().w;
			unsigned int pot_h = (unsigned)rootNode->GetRect().h;
			pot_w--;
			pot_w |= pot_w >> 1;
			pot_w |= pot_w >> 2;
			pot_w |= pot_w >> 4;
			pot_w |= pot_w >> 8;
			pot_w |= pot_w >> 16;
			pot_w++;
			pot_h--;
			pot_h |= pot_h >> 1;
			pot_h |= pot_h >> 2;
			pot_h |= pot_h >> 4;
			pot_h |= pot_h >> 8;
			pot_h |= pot_h >> 16;
			pot_h++;
			rootNode->SetRect(
				BinRect(
					rootNode->GetRect().id, 
					rootNode->GetRect().x, 
					rootNode->GetRect().y, 
					(float)pot_w, 
					(float)pot_h
				)
			);
		}
	}

	std::vector<BinRect> GetVolumes()
	{
		return volumes;
	}

	BinRect GetRootRect()
	{
		return rootNode->GetRect();
	}
private:
	std::vector<BinRect> volumes;
	BinNode* rootNode;
};

}

#endif