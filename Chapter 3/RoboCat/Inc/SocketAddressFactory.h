class SocketAddressFactory
{
public:
	static SocketAddressPtr CreateIPv4FromString(const string& inString);
	static SocketAddressPtr CreateIPv6FromString(const string& inString);
};