//**************************************************************************************************
//* FILE:		CBuffer.h
//*
//*



#ifndef _CBuffer_H_
#define _CBuffer_H_

#include <vector>


class CBuffer : public std::vector<unsigned char>
{
public:

	CBuffer()
	{

	}

	~CBuffer()
	{

	}

	void clear()
	{
		try
		{
			if (size() > 0)
			{
				resize(0);
			}
		}
		catch (...)
		{
		}
	}

	void assign(const CBuffer &rVal)
	{
		try
		{
			if (this->size() > 0)
			{
				this->clear();
			}

			if (rVal.size() > 0)
			{
				vector::assign(rVal.begin(), rVal.end());
			}
		}
		catch (...)
		{
		}
	}

	void assign(const std::string sVal)
	{
		try
		{
			if (this->size() > 0)
			{
				this->clear();
			}

			unsigned long nLen = (unsigned long) sVal.size();

			if (nLen > 0)
			{
				//vector::assign(sVal.begin(), sVal.end());

				//this->resize(nLen + 1);
				this->resize(nLen);

				memcpy((this->data()), (sVal.data()), nLen);
			}
			//else
			//{
			//	if (this->size() > 0)
			//	{
			//		this->clear();
			//	}
			//}
		}
		catch (...)
		{
		}
	}

	void assign(const char *pVal, int nLen)
	{
		try
		{
			if (this->size() > 0)
			{
				this->clear();
			}

			if (nLen < 1)
			{
				return;
			}

			//this->resize(nLen + 1);
			this->resize(nLen);

			memcpy((this->data()), pVal, nLen);
		}
		catch (...)
		{
		}
	}

	//CBuffer & operator=(const CBuffer &rVal)
	//{
	//	*this = (rVal);

	//	CBuffer &rRet = *this;

	//	return rRet;
	//}

	void operator=(const CBuffer *pVal)
	//CBuffer * operator=(const CBuffer *pVal)
	{
		if (this->size() > 0)
		{
			this->clear();
		}

		*this = (*pVal);

		//CBuffer *pRet = this;

		//return pRet;
	}

	//std::string operator=(std::string sVal)
	//{
	//	if (this->size() > 0)
	//	{
	//		this->clear();
	//	}

	//	vector::assign(sVal.begin(), sVal.end());

	//	return sVal;
	//}

	void operator=(std::string &sVal)
	//std::string operator=(std::string &sVal)
	{
		if (this->size() > 0)
		{
			this->clear();
		}

		vector::assign(sVal.begin(), sVal.end());

		//return sVal;
	}

	//std::string operator=(const std::string sVal)
	//{
	//	if (this->size() > 0)
	//	{
	//		this->clear();
	//	}

	//	vector::assign(sVal.begin(), sVal.end());

	//	return sVal;
	//}

	void operator=(const std::string &sVal)
	//std::string operator=(const std::string &sVal)
	{
		if (this->size() > 0)
		{
			this->clear();
		}

		vector::assign(sVal.begin(), sVal.end());

		//return sVal;
	}

	std::string getStr()
	{
		std::string sOut = "";

		unsigned long nLen = (unsigned long) this->size();

		if (nLen > 0)
		{
			sOut.assign(begin(), end());

			if (sOut[(nLen - 1)] != 0)
			{
				sOut.append(1, 0);
			}
		}

		return sOut;
	}

	//char * getPtr()
	//{
	//	char *pOut = 0;

	//	try
	//	{
	//		if (vector::size() > 0)
	//		{
	//			pOut = (char *) vector::data();
	//		}
	//	}
	//	catch (...)
	//	{
	//		return 0;
	//	}

	//	return pOut;
	//}

	void append(const CBuffer &rVal)
	{
		try
		{
			unsigned long nCurBufSize = (unsigned long) this->size();

			unsigned long nCopySize = (unsigned long) rVal.size();
			if (nCopySize > 0)
			{
				//* copy the new buffer data to the end of the current buffer data

				//unsigned long nNewSize = (nCurBufSize + nCopySize + 1);
				unsigned long nNewSize = (nCurBufSize + nCopySize);

				this->resize(nNewSize);

				unsigned char *pData = (this->data() + nCurBufSize);

				memcpy(pData, (rVal.data()), nCopySize);

				//* add a NULL char to the end of the buffer;

				//*(pData + (nNewSize - 1)) = 0;
			}
		}
		catch (...)
		{
		}
	}

	void append(std::string sVal)
	{
		try
		{
			unsigned long nCurBufSize = (unsigned long) this->size();

			unsigned long nCopySize = (unsigned long) sVal.size();
			if (nCopySize > 0)
			{
				//* copy the new buffer data to the end of the current buffer data

				//unsigned long nNewSize = (nCurBufSize + nCopySize + 1);
				unsigned long nNewSize = (nCurBufSize + nCopySize);

				this->resize(nNewSize);

				unsigned char *pData = (this->data() + nCurBufSize);

				memcpy(pData, (sVal.data()), nCopySize);

				//* add a NULL char to the end of the buffer;

				//*(pData + (nNewSize - 1)) = 0;
			}
		}
		catch (...)
		{
		}
	}

	void append(const char *pVal, int nLen)
	{
		if (nLen < 1)
		{
			return;
		}

		try
		{
			unsigned long nCurBufSize = (unsigned long) this->size();

			if (nLen > 0)
			{
				//* copy the new buffer data to the end of the current buffer data

				//unsigned long nNewSize = (nCurBufSize + nLen + 1);
				unsigned long nNewSize = (nCurBufSize + nLen);

				this->resize(nNewSize);

				unsigned char *pData = (this->data() + nCurBufSize);

				memcpy(pData, pVal, nLen);

				//* add a NULL char to the end of the buffer;

				//*(pData + (nNewSize - 1)) = 0;
			}
		}
		catch (...)
		{
		}
	}

	void insert(unsigned long nPos, const unsigned char cVal, int nLen)
	{
		if (nLen < 1)
		{
			return;
		}

		try
		{
			//* if the buffer is currently too small... pad it with NULL bytes
			
			for (int x = (int)size(); x < (int) nPos; x++)
			{
				push_back((unsigned char) 0);
			}

			//* insert bytes into buffer

			for (int b = 0; b < nLen; b++)
			{
				vector::insert((begin() + (nPos + b)), cVal);
			}
		}
		catch (...)
		{
		}
	}

	void insert(unsigned long nPos, const char *pVal, int nLen)
	{
		if (nLen < 1)
		{
			return;
		}

		try
		{
			//* if the buffer is currently too small... pad it with NULL bytes

			for (int x = (int) size(); x < (int) nPos; x++)
			{
				this->push_back((unsigned char) 0);
			}

			//* insert bytes into buffer

			for (int b = 0; b < nLen; b++)
			{
				vector::insert((begin() + (nPos + b)), ((unsigned char) *(pVal + b)));
			}
		}
		catch (...)
		{
		}
	}

};


#endif
