#include <cstdlib>
#include <cstdint>
#include <type_traits>

#define STREAM_ENDIANNESS 0
#define PLATFORM_ENDIANNESS 0

class GameObject;
class LinkingContext;

template <typename Stream>
class MemoryStream
{
public:
	template<typename T>
	void Serialize(T& ioData)
	{
		static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Generic Serialize only supports primitive data types");

		if (STREAM_ENDIANNESS == PLATFORM_ENDIANNESS)
		{
			static_cast<Stream&>(*this).Serialize_(&ioData, sizeof(ioData));
		}
		else
		{
			if (static_cast<Stream&>(*this).IsInput())
			{
				T data;
				static_cast<Stream&>(*this).Serialize_(&data, sizeof(T));
				ioData = ByteSwap(data);
			}
			else
			{
				T swappedData = ByteSwap(ioData);
				static_cast<Stream&>(*this).Serialize_(&swappedData, sizeof(swappedData));
			}
		}
	}
};

class OutputMemoryStream : public MemoryStream<OutputMemoryStream>
{
public:
	OutputMemoryStream() : mLinkingContext(nullptr), mBuffer(), mHead(), mCapacity() { ReallocBuffer(32); }

	~OutputMemoryStream()
	{
		std::free(mBuffer);
	}

	//get a pointer to the data in the stream
	char* GetBufferPtr() const { return mBuffer; }
	uint32_t GetLength() const { return mHead; }

	void Write(const void* inData, size_t inByteCount);

	template<typename T> void Write(T inData)
	{
		static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Generic Write only supports primitive data types");

		if (STREAM_ENDIANNESS == PLATFORM_ENDIANNESS)
		{
			Write(&inData, sizeof(inData));
		}
		else
		{
			T swappedData = ByteSwap(inData);
			Write(&swappedData, sizeof(swappedData));
		}
	}

	//
	// Map
	//

	void Write(const unordered_map<int, int>& inMap)
	{
		size_t elementCount = inMap.size();
		Write(elementCount);
		for (const std::pair<int, int>& element : inMap)
		{
			Write(element.first);
			Write(element.second);
		}
	}

	template <typename tKey, typename tValue>
	void Write(const unordered_map<tKey, tValue>& inMap)
	{
		size_t elementCount = inMap.size();
		Write(elementCount);
		for (const std::pair<tKey, tValue>& element : inMap)
		{
			Write(element.first);
			Write(element.second);
		}
	}

	//
	// Vector
	//

	void Write(const std::vector<int>& inIntVector)
	{
		size_t elementCount = inIntVector.size();
		Write(elementCount);
		Write(inIntVector.data(), elementCount * sizeof(int));
	}
	
	template<typename T>
	void Write(const std::vector<T>& inVector)
	{
		uint32_t elementCount = inVector.size();
		Write(elementCount);
		for(const T& element : inVector)
		{
			Write(element);
		}
	}
	
	void Write(const std::string& inString)
	{
		size_t elementCount = inString.size() ;
		Write(elementCount);
		Write(inString.data(), elementCount * sizeof(char));
	}
	
	void Write(const GameObject* inGameObject)
	{
		uint32_t networkId = mLinkingContext->GetNetworkId(const_cast<GameObject*>(inGameObject), false);
		Write(networkId);
	}

	void Serialize_(void* ioData, uint32_t inByteCount)
	{
		Write(ioData, inByteCount);
	}

	bool IsInput() const { return false; }

private:
	void ReallocBuffer(uint32_t inNewLength);
	
	char* mBuffer;
	uint32_t mHead;
	uint32_t mCapacity;
	
	LinkingContext* mLinkingContext;
};

class InputMemoryStream : public MemoryStream<InputMemoryStream>
{
public:
	InputMemoryStream(char* inBuffer, uint32_t inByteCount) : 
		mBuffer(inBuffer), 
		mCapacity(inByteCount), 
		mHead(0), 
		mLinkingContext(nullptr) 
	{}

	~InputMemoryStream() 
	{ 
		std::free(mBuffer);
	}
		
	uint32_t GetRemainingDataSize() const { return mCapacity - mHead; }
	
	void Read(void* outData, uint32_t inByteCount);
	
	//
	// Map
	//

	void Read(unordered_map<int, int>& outMap)
	{
		size_t elementCount;
		Read(elementCount);
		outMap.reserve(elementCount);
		for (int i = 0; i < elementCount; i++)
		{
			std::pair<int, int> element;
			Read(element.first);
			Read(element.second);
			outMap.insert(element);
		}
	}

	template <typename tKey, typename tValue>
	void Read(unordered_map<tKey, tValue>& outMap)
	{
		size_t elementCount;
		Read(elementCount);
		outMap.reserve(elementCount);
		for (int i = 0; i < elementCount; i++)
		{
			std::pair<tKey, tValue> element;
			Read(element.first);
			Read(element.second);
			outMap.insert(element);
		}
	}

	//
	// Vector
	//
	
	template<typename T>
	void Read(std::vector<T>& outVector)
	{
		size_t elementCount;
		Read(elementCount);
		outVector.resize(elementCount);
		for(T& element : outVector)
		{
			Read(element);
		}
	}

	template<typename T> void Read(T& outData)
	{
		static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Generic Read only supports primitive data types");
		Read(&outData, sizeof(outData));
	}
	
	void Read(GameObject*& outGameObject)
	{
		uint32_t networkId;
		Read(networkId);
		outGameObject = mLinkingContext->GetGameObject(networkId);
	}

	void Serialize_(void* ioData, uint32_t inByteCount)
	{
		Read(ioData, inByteCount);
	}
	
	bool IsInput() const { return true; }
	
private:
	char* mBuffer;
	uint32_t mHead;
	uint32_t mCapacity;

	LinkingContext* mLinkingContext;
};
