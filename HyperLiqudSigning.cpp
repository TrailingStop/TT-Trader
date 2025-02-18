#include <openssl/evp.h>



static const char* s_strJsonDomainStruct = R"({
	"EIP712Domain": [
		{ "name": "name", "type" : "string"},
		{ "name": "version", "type" : "string" },
		{ "name": "chainId", "type" : "uint256" },
		{ "name": "verifyingContract", "type" : "address" }
		]
	}
)";

static const char* s_strJsonDomainValuesL1 = R"({
	"values": {
		"name": "Exchange",
		"version": "1",
		"chainId": 1337,
		"verifyingContract": "0x0000000000000000000000000000000000000000"
		}
	}
)";


static const char* s_strJsonAgentStruct = R"({
	"Agent": [
		{ "name": "source", "type" : "string"},
		{ "name": "connectionId", "type" : "bytes32" }
		]
	}
)";








void CExchgWebSocketBase::_PackMessageString(const char* p_strString, std::vector<uint8_t>& p_vectorPackedMessage)
{
	size_t l_stStringLength = strlen(p_strString);

	while (true)
	{
		if (l_stStringLength < 32)
		{
			uint8_t l_ui8Value = 0xA0 | (l_stStringLength & 0x1F);
			p_vectorPackedMessage.push_back(l_ui8Value);
			break;
		}

		if (l_stStringLength < 256)
		{
			p_vectorPackedMessage.push_back(0xd9);
			p_vectorPackedMessage.push_back(l_stStringLength & 0xFF);
			break;
		}
		_ASSERT(false);
		return;
	}

	for (int32_t l_nIndex = 0; l_nIndex < l_stStringLength; ++l_nIndex)
	{
		p_vectorPackedMessage.push_back(p_strString[l_nIndex]);
	}
}




void CExchgWebSocketBase::_PackMessageArray(const rapidjson::Value& p_valArray, std::vector<uint8_t>& p_vectorPackedMessage)
{
	_ASSERT(p_valArray.IsArray());

	size_t l_stArrayLength = p_valArray.Size();
	_ASSERT(l_stArrayLength < 16);

	uint8_t l_ui8Value = 0x90 | (l_stArrayLength & 0x0F);
	p_vectorPackedMessage.push_back(l_ui8Value);

	for (const auto& l_iValArray : p_valArray.GetArray())
	{
		_PackMessageValue(l_iValArray, p_vectorPackedMessage);
	}
}


void CExchgWebSocketBase::_PackMessageObject(const rapidjson::Value& p_valObject, std::vector<uint8_t>& p_vectorPackedMessage)
{
	DebugJsonValue(p_valObject);
	_ASSERT(p_valObject.IsObject());

	size_t l_stObjectLength = p_valObject.GetObj().MemberCount();
	_ASSERT(l_stObjectLength < 16);

	uint8_t l_ui8Value = 0x80 | (l_stObjectLength & 0x0F);
	p_vectorPackedMessage.push_back(l_ui8Value);


	// DebugString( "X_PackMessageObject:" );
	// DebugMemoryBlock( p_vectorPackedMessage.data(), p_vectorPackedMessage.size() );

	for (const auto& l_iValObject : p_valObject.GetObj())
	{
		CLightDynString l_strName = l_iValObject.name;
		DebugString(l_strName);

		_PackMessageString((const char*)l_strName, p_vectorPackedMessage);
		DebugString("X_PackMessageObject0:");
		DebugMemoryBlock(p_vectorPackedMessage.data(), p_vectorPackedMessage.size());
		_PackMessageValue(l_iValObject.value, p_vectorPackedMessage);
		DebugString("X_PackMessageObject1:");
		DebugMemoryBlock(p_vectorPackedMessage.data(), p_vectorPackedMessage.size());
	}
}



void CExchgWebSocketBase::_PackMessageValue(const rapidjson::Value& p_valValue, std::vector<uint8_t>& p_vectorPackedMessage)
{
	if (rapidjson::kObjectType == p_valValue.GetType())
	{
		_PackMessageObject(p_valValue, p_vectorPackedMessage);
		return;
	}

	if (rapidjson::kArrayType == p_valValue.GetType())
	{
		_PackMessageArray(p_valValue, p_vectorPackedMessage);
		return;
	}

	if (rapidjson::kStringType == p_valValue.GetType())
	{
		CLightDynString l_strValue = p_valValue;
		_PackMessageString((const char*)l_strValue, p_vectorPackedMessage);
		return;
	}

	if (rapidjson::kFalseType == p_valValue.GetType())
	{
		p_vectorPackedMessage.push_back(0xc2);
		return;
	}

	if (rapidjson::kTrueType == p_valValue.GetType())
	{
		p_vectorPackedMessage.push_back(0xc3);
		return;
	}

	if (rapidjson::kNumberType == p_valValue.GetType())
	{
		int64_t l_i64Value = p_valValue.GetInt64();
		if (l_i64Value < 0x80)
		{
			p_vectorPackedMessage.push_back(l_i64Value & 0x7F);
			return;
		}
		if (l_i64Value < 0x100)
		{
			p_vectorPackedMessage.push_back(0xcc);
			p_vectorPackedMessage.push_back(l_i64Value & 0xFF);
			return;
		}
		if (l_i64Value < 0x10000)
		{
			p_vectorPackedMessage.push_back(0xcd);
			p_vectorPackedMessage.push_back((l_i64Value >> (1 << 3)) & 0xFF);
			p_vectorPackedMessage.push_back((l_i64Value >> (0 << 3)) & 0xFF);
			return;
		}
		if (l_i64Value < 0x100000000)
		{
			p_vectorPackedMessage.push_back(0xce);
			p_vectorPackedMessage.push_back((l_i64Value >> (3 << 3)) & 0xFF);
			p_vectorPackedMessage.push_back((l_i64Value >> (2 << 3)) & 0xFF);
			p_vectorPackedMessage.push_back((l_i64Value >> (1 << 3)) & 0xFF);
			p_vectorPackedMessage.push_back((l_i64Value >> (0 << 3)) & 0xFF);
			return;
		}
		p_vectorPackedMessage.push_back(0xcf);
		p_vectorPackedMessage.push_back((l_i64Value >> (7 << 3)) & 0xFF);
		p_vectorPackedMessage.push_back((l_i64Value >> (6 << 3)) & 0xFF);
		p_vectorPackedMessage.push_back((l_i64Value >> (5 << 3)) & 0xFF);
		p_vectorPackedMessage.push_back((l_i64Value >> (4 << 3)) & 0xFF);
		p_vectorPackedMessage.push_back((l_i64Value >> (3 << 3)) & 0xFF);
		p_vectorPackedMessage.push_back((l_i64Value >> (2 << 3)) & 0xFF);
		p_vectorPackedMessage.push_back((l_i64Value >> (1 << 3)) & 0xFF);
		p_vectorPackedMessage.push_back((l_i64Value >> (0 << 3)) & 0xFF);
		return;
	}

	DebugInt(p_valValue.GetType());
	_ASSERT(false);
}






void CExchgWebSocketBase::_SignMsg_GetHashFromValue(const CLightDynString& p_strType, const rapidjson::Value& p_valValue, hash32_t& p_h32Return)
{
	if (p_strType.IsTheSameIgnoreCase("string"))
	{
		CLightDynString l_strValue = p_valValue;
		_keccak((char*)l_strValue, l_strValue.GetLength(), p_h32Return);
		return;
	}

	if (p_strType.IsTheSameIgnoreCase("uint256"))
	{
		uint256_type l_ui256Value = p_valValue.GetUint64();
		memcpy(&p_h32Return, l_ui256Value.m_uint256.ui8, sizeof(uint256_type));
		return;
	}

	if (p_strType.IsTheSameIgnoreCase("address"))
	{
		CLightDynString l_strValue = p_valValue;

		const char* l_strAddress = (const char*)l_strValue;
		if ('x' == l_strAddress[1])
		{
			l_strAddress += 2;
		}

		memset((uint8_t*)&p_h32Return, 0, sizeof(hash32_t));
		HexString2Memory(l_strAddress, strlen(l_strAddress), (uint8_t*)&p_h32Return, sizeof(hash32_t));
		return;
	}

	if (p_strType.IsTheSameIgnoreCase("bytes32"))
	{
		CLightDynString l_strValue = p_valValue;
		HexString2Memory(l_strValue, l_strValue.GetLength(), (uint8_t*)&p_h32Return, sizeof(hash32_t));
		return;
	}

	_ASSERT(false);
}




void CExchgWebSocketBase::_SignMsg_GetHashFromStructure(const char* p_strName, const rapidjson::Value& p_valStructureArray, hash32_t& p_h32Return)
{
	_ASSERT(p_valStructureArray.IsArray());
	_ASSERT(p_valStructureArray.Size() > 0);

	CLightDynString l_strPackedStruct = p_strName;
	l_strPackedStruct.Append("(");
	bool l_fAddSeparator = false;

	for (const auto& l_iItem : p_valStructureArray.GetArray())
	{
		if (l_fAddSeparator)
		{
			l_strPackedStruct.Append(",");
		}

		const auto& l_valItemName = l_iItem.FindMember("name");
		CLightDynString l_strName = l_valItemName->value;
		const auto& l_valItemType = l_iItem.FindMember("type");
		CLightDynString l_strType = l_valItemType->value;

		l_strPackedStruct.AppendFormat("%s %s", (char*)l_strType, (char*)l_strName);
		l_fAddSeparator = true;
	}
	l_strPackedStruct.Append(")");

	_keccak((char*)l_strPackedStruct, l_strPackedStruct.GetLength(), p_h32Return);
}



void CExchgWebSocketBase::_SignMsg_PackJsonStructure(const rapidjson::Document& p_docStructure, const rapidjson::Document& p_docValues, hash32_t& p_h32Return)
{
	const auto& l_valMainObject = p_docStructure.MemberBegin();
	_ASSERT(l_valMainObject->value.IsArray());



	hash32_t* l_pHashes = new hash32_t[l_valMainObject->value.GetArray().Size() + 1];
	SCOPE_EXIT()
	{
		delete[] l_pHashes;
	};
	size_t l_stHashIndex = 0;


	_SignMsg_GetHashFromStructure(l_valMainObject->name.GetString(), l_valMainObject->value, l_pHashes[l_stHashIndex++]);



	// calculate the value hashes
	{
		const auto& l_valValuesObject = p_docValues.MemberBegin();
		_ASSERT(l_valValuesObject->value.IsObject());

		const auto& l_valDomainValues = l_valValuesObject->value.GetObj();

		for (const auto& l_iItem : l_valMainObject->value.GetArray())
		{
			const auto& l_valItemName = l_iItem.FindMember("name");
			CLightDynString l_strName = l_valItemName->value;
			const auto& l_valItemType = l_iItem.FindMember("type");
			CLightDynString l_strType = l_valItemType->value;

			DebugString(l_strName);
			DebugString(l_strType);


			const auto& l_valValue = l_valDomainValues.FindMember((char*)l_strName);
			_SignMsg_GetHashFromValue(l_strType, l_valValue->value, l_pHashes[l_stHashIndex++]);
		}
	}

	for (int i = 0; i < l_stHashIndex; i++)
	{
		DebugMemoryBlock(&l_pHashes[i], sizeof(hash32_t));
	}

	_keccak((char*)l_pHashes, l_stHashIndex * sizeof(hash32_t), p_h32Return);
}





CExchgWebSocketBase::_EXCHG_SIGNATURE CExchgWebSocketBase::sign_l1_actionX(const rapidjson::Value& p_valValue, const char* p_strVault, uint64_t p_ui64None, bool p_fMainNet)
{
	uint8_t l_arraySign[1 + 1 + sizeof(hash32_t) + sizeof(hash32_t)];
	int32_t l_i32NextSignIndex = 0;
	l_arraySign[l_i32NextSignIndex++] = 0x19;
	l_arraySign[l_i32NextSignIndex++] = 0x01;

	{
		hash32_t l_h32Message;
		_SignMsg_GetHashMessage(p_valValue, p_strVault, p_ui64None, l_h32Message);
		DebugString("l_h32Message:");
		DebugMemoryBlock((uint8_t*)&l_h32Message, sizeof(hash32_t));


		hash32_t l_h32Domain;
		_SignMsg_GetHashDomain(l_h32Domain);
		memcpy(&l_arraySign[l_i32NextSignIndex], &l_h32Domain, sizeof(hash32_t));
		l_i32NextSignIndex += sizeof(hash32_t);
		DebugString("l_h32Domain:");
		DebugMemoryBlock((uint8_t*)&l_h32Domain, sizeof(hash32_t));

		hash32_t l_h32L1;
		_SignMsg_GetHashL1(l_h32Message, l_h32L1, p_fMainNet);
		memcpy(&l_arraySign[l_i32NextSignIndex], &l_h32L1, sizeof(hash32_t));
		l_i32NextSignIndex += sizeof(hash32_t);
		DebugString("l_h32L1:");
		DebugMemoryBlock((uint8_t*)&l_h32L1, sizeof(hash32_t));
	}

	DebugString("joined:");
	DebugMemoryBlock(l_arraySign, l_i32NextSignIndex);

	hash32_t l_h32Sign;
	_keccak((char*)l_arraySign, l_i32NextSignIndex, l_h32Sign);
	DebugString("keccak(joined):");
	DebugMemoryBlock((uint8_t*)&l_h32Sign, sizeof(hash32_t));






	// create z
	CLightDynString l_strConvert;
	l_strConvert.MemoryToHex((uint8_t*)&l_h32Sign, sizeof(hash32_t), true);
	boost::multiprecision::int1024_t l_i1024Z{ (char*)l_strConvert };
	{
		std::stringstream ss;
		ss << "Z: " << std::hex << l_i1024Z;
		DebugString((char*)ss.str().c_str());
	}






	// create k
	hash32_t l_h32K;
	_SignMsg_GenerateK(l_h32Sign, l_h32K);


	l_strConvert.MemoryToHex((uint8_t*)&l_h32K, sizeof(hash32_t), true);
	boost::multiprecision::int256_t l_i256K{ (char*)l_strConvert };
	{
		std::stringstream ss;
		ss << "K: " << std::hex << l_i256K;
		DebugString((char*)ss.str().c_str());
	}





	boost::multiprecision::int256_t l_i256G[3];
	l_i256G[0] = boost::multiprecision::int256_t{ "55066263022277343669578718895168534326250603453777594175500187360389116729240" };
	l_i256G[1] = boost::multiprecision::int256_t{ "32670510020758816978083085130507043184471273380659243275938904335757337482424" };
	l_i256G[2] = 1;

	_JacobianFastMultiply(l_i256G, l_i256K);


	{
		std::stringstream ss;
		ss << "R: " << std::hex << l_i256G[0];
		DebugString((char*)ss.str().c_str());
	}
	{
		std::stringstream ss;
		ss << "Y: " << std::hex << l_i256G[1];
		DebugString((char*)ss.str().c_str());
	}


	l_strConvert.MemoryToHex((uint8_t*)&m_h32PrivateKey, sizeof(hash32_t), true);
	boost::multiprecision::int1024_t l_i1024PrivateKey{ (char*)l_strConvert };

	boost::multiprecision::int1024_t l_i256Sraw = _JacobianInv(l_i256K, m_N) * (l_i1024Z + l_i1024PrivateKey * l_i256G[0]) % m_N;

	bool l_fRawIsLessThanN = (l_i256Sraw << 1) < m_N;
	boost::multiprecision::int1024_t l_i1024SrawXOR = l_fRawIsLessThanN ? 0 : 1;
	boost::multiprecision::int1024_t l_i1024SrawV = 27 + ((l_i256G[1] % 2) ^ l_i1024SrawXOR);

	if (!l_fRawIsLessThanN)
	{
		l_i256Sraw = m_N - l_i256Sraw;
	}


	_EXCHG_SIGNATURE l_sig;
	l_sig.i32V = (int32_t)l_i1024SrawV;
	{
		std::stringstream ss;
		ss << std::hex << l_i256G[0];
		strcpy_s(l_sig.strR, ss.str().c_str());
	}
	{
		std::stringstream ss;
		ss << std::hex << l_i256Sraw;
		strcpy_s(l_sig.strS, ss.str().c_str());
	}

	return l_sig;
}




int32_t CExchgWebSocketBase::_SignMsg_GenerateKMessage(uint8_t* p_arrayMessage, const hash32_t& p_h32Header, uint8_t p_ui8Delimizer, const hash32_t& p_h32Sign)
{
	int32_t l_i32MessageIndex = 0;

	// copy header
	memcpy(&p_arrayMessage[l_i32MessageIndex], &p_h32Header, sizeof(hash32_t));
	l_i32MessageIndex += sizeof(hash32_t);

	// add delimiter
	p_arrayMessage[l_i32MessageIndex++] = p_ui8Delimizer;

	// copy private key
	// need to create the private key hash?
	if (m_fInitPrivateKeyHash)
	{
		m_fInitPrivateKeyHash = false;

		// create private key hash
		const char* l_strPrivateKey = m_pInfoConfig->strSecretKey;
		if ('x' == l_strPrivateKey[1])
		{
			l_strPrivateKey += 2;
		}
		HexString2Memory((char*)l_strPrivateKey, strlen(l_strPrivateKey), (uint8_t*)&m_h32PrivateKey, sizeof(hash32_t));
	}

	memcpy(&p_arrayMessage[l_i32MessageIndex], &m_h32PrivateKey, sizeof(hash32_t));
	l_i32MessageIndex += sizeof(hash32_t);

	// copy message hash
	memcpy(&p_arrayMessage[l_i32MessageIndex], &p_h32Sign, sizeof(hash32_t));
	l_i32MessageIndex += sizeof(hash32_t);

	return l_i32MessageIndex;
}



void CExchgWebSocketBase::_SignMsg_GenerateK(const hash32_t& p_h32Sign, hash32_t& p_h32Result)
{
	CLightDynString l_strMessage;
	l_strMessage.MemoryToHex((uint8_t*)&p_h32Sign, sizeof(hash32_t), true);
	DebugString(l_strMessage);

	boost::multiprecision::int256_t l_i256X{ (char*)l_strMessage };



	uint8_t l_arrayMessage[sizeof(hash32_t) + 1 + sizeof(hash32_t) + sizeof(hash32_t)];

	hash32_t l_h32V0;
	memset(&l_h32V0, 0x01, sizeof(hash32_t));

	int32_t l_i32MessageLength = _SignMsg_GenerateKMessage(l_arrayMessage, l_h32V0, 0, p_h32Sign);


	hash32_t l_h32K0;
	memset(&l_h32K0, 0, sizeof(hash32_t));

	hash32_t l_h32K1;
	_sha256(l_arrayMessage, l_i32MessageLength, (uint8_t*)&l_h32K0, sizeof(hash32_t), l_h32K1);

	hash32_t l_h32V1;
	_sha256(l_h32V0, sizeof(hash32_t), (uint8_t*)&l_h32K1, sizeof(hash32_t), l_h32V1);





	l_i32MessageLength = _SignMsg_GenerateKMessage(l_arrayMessage, l_h32V1, 1, p_h32Sign);

	hash32_t l_h32K2;
	_sha256(l_arrayMessage, l_i32MessageLength, (uint8_t*)&l_h32K1, sizeof(hash32_t), l_h32K2);

	hash32_t l_h32V2;
	_sha256(l_h32V1, sizeof(hash32_t), (uint8_t*)&l_h32K2, sizeof(hash32_t), l_h32V2);

	_sha256(l_h32V2, sizeof(hash32_t), (uint8_t*)&l_h32K2, sizeof(hash32_t), p_h32Result);
}



void CExchgWebSocketBase::_SignMsg_AppendNonce(std::vector<uint8_t>& p_vectorPackedMessage, uint64_t p_ui64None)
{
	// append nonce....
	const uint8_t* l_pNonce = (const uint8_t*)&p_ui64None;
	for (int32_t l_nIndex = sizeof(p_ui64None) - 1; l_nIndex >= 0; l_nIndex--)
	{
		p_vectorPackedMessage.push_back(l_pNonce[l_nIndex]);
	}
}




void CExchgWebSocketBase::_SignMsg_AppendVault(std::vector<uint8_t>& p_vectorPackedMessage, const char* p_strVault)
{
	if (nullptr == p_strVault)
	{
		p_vectorPackedMessage.push_back(0);
		return;
	}

	p_vectorPackedMessage.push_back(1);
	for (int32_t l_nIndex = 0;; l_nIndex++)
	{
		if (0 == p_strVault[l_nIndex])
		{
			break;
		}
		p_vectorPackedMessage.push_back(p_strVault[l_nIndex]);
	}
}


void CExchgWebSocketBase::_SignMsg_GetHashMessage(const rapidjson::Value& p_valValue, const char* p_strVault, uint64_t p_ui64None, hash32_t& p_h32Return)
{
	std::vector<uint8_t> l_vectorPackedMessage;
	_PackMessageObject(p_valValue, l_vectorPackedMessage);

	DebugString("_PackMessageObject:");
	DebugMemoryBlock(l_vectorPackedMessage.data(), l_vectorPackedMessage.size());



	_SignMsg_AppendNonce(l_vectorPackedMessage, p_ui64None);
	DebugString("_SignMsg_AppendNonce:");
	DebugMemoryBlock(l_vectorPackedMessage.data(), l_vectorPackedMessage.size());

	_SignMsg_AppendVault(l_vectorPackedMessage, p_strVault);
	DebugString("_SignMsg_AppendVault:");
	DebugMemoryBlock(l_vectorPackedMessage.data(), l_vectorPackedMessage.size());

	_keccak(l_vectorPackedMessage.data(), l_vectorPackedMessage.size(), p_h32Return);
}


void CExchgWebSocketBase::_SignMsg_GetHashDomain(hash32_t& p_h32Return)
{
	rapidjson::Document l_docStructure;
	l_docStructure.Parse<rapidjson::kParseNoFlags>(s_strJsonDomainStruct);
	_ASSERT(!l_docStructure.GetParseError());
	_ASSERT(l_docStructure.MemberCount() > 0);

	rapidjson::Document l_docValues;
	l_docValues.Parse<rapidjson::kParseNoFlags>(s_strJsonDomainValuesL1);
	_ASSERT(!l_docValues.GetParseError());
	_ASSERT(l_docValues.MemberCount() > 0);

	_SignMsg_PackJsonStructure(l_docStructure, l_docValues, p_h32Return);
}



void CExchgWebSocketBase::_SignMsg_GetHashL1(const hash32_t& p_h32Domain, hash32_t& p_h32Return, bool p_fMainNet)
{
	rapidjson::Document l_docStructure;
	l_docStructure.Parse<rapidjson::kParseNoFlags>(s_strJsonAgentStruct);
	_ASSERT(!l_docStructure.GetParseError());
	_ASSERT(l_docStructure.MemberCount() > 0);

	rapidjson::Document l_docValues;
	rapidjson::Document::AllocatorType& l_DocumentAllocator = l_docValues.GetAllocator();
	l_docValues.SetObject();
	{
		rapidjson::Value l_valAgent(rapidjson::kObjectType);
		l_valAgent.AddMember("source", p_fMainNet ? "a" : "b", l_DocumentAllocator);

		CLightDynString l_strHash = Memory2HexString((const uint8_t*)&p_h32Domain, sizeof(hash32_t));
		l_valAgent.AddMember("connectionId", rapidjson::Value((const char*)l_strHash, l_DocumentAllocator), l_DocumentAllocator);

		l_docValues.AddMember("value", l_valAgent, l_DocumentAllocator);
	}

	_SignMsg_PackJsonStructure(l_docStructure, l_docValues, p_h32Return);
}


void CExchgWebSocketBase::_keccak(const void* p_pData, size_t p_stLength, hash32_t& p_h32Return)
{
	sph_keccak_context l_context;
	sph_keccak256_init(&l_context);
	sph_keccak256(&l_context, (uint8_t*)p_pData, p_stLength);
	sph_keccak256_close(&l_context, &p_h32Return);
}



void CExchgWebSocketBase::_sha256(const uint8_t* p_pMessage, size_t p_stMessageLength, const uint8_t* p_pKey, size_t p_stKeyLength, hash32_t& p_h32Return)
{
	EVP_MD_CTX* l_pOpenSslContect = EVP_MD_CTX_new();
	EVP_PKEY* l_pOpenSslKey = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, p_pKey, p_stKeyLength);

	EVP_DigestSignInit(l_pOpenSslContect, NULL, EVP_sha256(), NULL, l_pOpenSslKey);
	EVP_DigestSignUpdate(l_pOpenSslContect, p_pMessage, p_stMessageLength);

	size_t l_ui32ResultLen = sizeof(hash32_t);
	EVP_DigestSignFinal(l_pOpenSslContect, (uint8_t*)&p_h32Return, &l_ui32ResultLen);
	EVP_MD_CTX_free(l_pOpenSslContect);
}





boost::multiprecision::int1024_t CExchgWebSocketBase::_JacobianFloorDiv(const boost::multiprecision::int1024_t& p_A, const boost::multiprecision::int1024_t& p_N)
{
	_ASSERT(0 != p_N);

	boost::multiprecision::int1024_t l_i1024Div = p_A / p_N;
	if (p_A >= 0)
	{
		return l_i1024Div;
	}

	if (0 != (p_A % p_N))
	{
		l_i1024Div--;
	}
	return l_i1024Div;
}


boost::multiprecision::int1024_t CExchgWebSocketBase::_JacobianFloorMod(const boost::multiprecision::int1024_t& p_A, const boost::multiprecision::int1024_t& p_N)
{
	boost::multiprecision::int1024_t l_i1024Mod = p_A % p_N;
	if (p_A >= 0)
	{
		return l_i1024Mod;
	}

	if (0 == l_i1024Mod)
	{
		return  l_i1024Mod;
	}

	return p_A - (_JacobianFloorDiv(p_A, p_N) * p_N);
}



boost::multiprecision::int1024_t CExchgWebSocketBase::_JacobianInv(const boost::multiprecision::int1024_t& p_A, const boost::multiprecision::int1024_t& p_N)
{
	if (0 == p_A)
	{
		return 0;
	}

	boost::multiprecision::int1024_t l_hm = 0;
	boost::multiprecision::int1024_t l_lm = 1;
	boost::multiprecision::int1024_t l_high = p_N;
	boost::multiprecision::int1024_t l_low = _JacobianFloorMod(p_A, p_N);

	while (l_low > 1)
	{
		boost::multiprecision::int1024_t l_r = _JacobianFloorDiv(l_high, l_low);
		// boost::multiprecision::int256_t l_r = l_high / l_low;//
		boost::multiprecision::int1024_t l_new = l_high - l_low * l_r;
		boost::multiprecision::int1024_t l_nm = l_hm - l_lm * l_r;

		l_high = l_low;
		l_hm = l_lm;
		l_low = l_new;
		l_lm = l_nm;
	}

	return _JacobianFloorMod(l_lm, p_N);

	/*
	def inv( a: int, n : int ) -> int:
	if a == 0 :
		return 0
		lm, hm = 1, 0
		low, high = a % n, n
		while low > 1:
		r = high // low
		nm, new = hm - lm * r, high - low * r
		lm, low, hm, high = nm, new, lm, low
		return lm % n
	*/
}


void CExchgWebSocketBase::_JacobianFrom(boost::multiprecision::int1024_t* p_pA)
{
	boost::multiprecision::int1024_t l_z = _JacobianInv(p_pA[2], m_P);
	{
		boost::multiprecision::uint1024_t l_Print = (boost::multiprecision::uint1024_t)l_z;
		std::stringstream ss;
		ss << "l_z: " << std::hex << l_Print;
		DebugString((char*)ss.str().c_str());
	}

	boost::multiprecision::int1024_t l_nx = _JacobianFloorMod(p_pA[0] * l_z * l_z, m_P);
	boost::multiprecision::int1024_t l_ny = _JacobianFloorMod(p_pA[1] * l_z * l_z * l_z, m_P);

	p_pA[0] = l_nx;
	p_pA[1] = l_ny;


	/*
	def from_jacobian( p: Tuple[int, int, int] )->Tuple[int, int]:
	z = inv( p[2], P )
		return ( ( p[0] * z * *2 ) % P, ( p[1] * z * *3 ) % P )
	*/
}


void CExchgWebSocketBase::_JacobianAdd(boost::multiprecision::int1024_t* p_pA, const boost::multiprecision::int1024_t* p_pB)
{
	if (0 == p_pA[1])
	{
		p_pA[0] = p_pB[0];
		p_pA[1] = p_pB[1];
		p_pA[2] = p_pB[2];
		return;
	}

	if (0 == p_pB[1])
	{
		return;
	}

	boost::multiprecision::int1024_t l_ui1024CopyA[3];
	l_ui1024CopyA[0] = p_pA[0];
	l_ui1024CopyA[1] = p_pA[1];
	l_ui1024CopyA[2] = p_pA[2];

	boost::multiprecision::int1024_t l_ui1024CopyB[3];
	l_ui1024CopyB[0] = p_pB[0];
	l_ui1024CopyB[1] = p_pB[1];
	l_ui1024CopyB[2] = p_pB[2];


	boost::multiprecision::int1024_t l_i1024U1 = _JacobianFloorMod(l_ui1024CopyA[0] * l_ui1024CopyB[2] * l_ui1024CopyB[2], m_P);
	/*
	{
		std::stringstream ss;
		ss << l_i512U1;
		DebugString( (char*)ss.str().c_str() );
	}
	*/

	boost::multiprecision::int1024_t l_i1024U2 = _JacobianFloorMod(l_ui1024CopyB[0] * l_ui1024CopyA[2] * l_ui1024CopyA[2], m_P);
	/*
	{
		std::stringstream ss;
		ss << l_i512U2;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	boost::multiprecision::int1024_t l_i1024S1 = _JacobianFloorMod(l_ui1024CopyA[1] * l_ui1024CopyB[2] * l_ui1024CopyB[2] * l_ui1024CopyB[2], m_P);
	/*
	{
		std::stringstream ss;
		ss << l_i512S1;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	boost::multiprecision::int1024_t l_i1024S2 = _JacobianFloorMod(l_ui1024CopyB[1] * l_ui1024CopyA[2] * l_ui1024CopyA[2] * l_ui1024CopyA[2], m_P);
	/*
	{
		std::stringstream ss;
		ss << l_i512S2;
		DebugString( (char*)ss.str().c_str() );
	}
	*/


	if (l_i1024U1 == l_i1024U2)
	{
		if (l_i1024S1 != l_i1024S2)
		{
			p_pA[0] = 0;
			p_pA[1] = 0;
			p_pA[2] = 1;
			return;
		}
		_JacobianDouble(p_pA);
		return;
	}


	boost::multiprecision::int1024_t l_i1024H = l_i1024U2 - l_i1024U1;
	/*
	{
		std::stringstream ss;
		ss << l_i512H;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	boost::multiprecision::int1024_t l_i1024R = l_i1024S2 - l_i1024S1;
	/*
	{
		std::stringstream ss;
		ss << l_i512R;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	boost::multiprecision::int1024_t l_i1024H2 = _JacobianFloorMod(l_i1024H * l_i1024H, m_P);
	/*
	{
		std::stringstream ss;
		ss << l_i512H2;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	boost::multiprecision::int1024_t l_i1024H3 = _JacobianFloorMod(l_i1024H * l_i1024H2, m_P);
	/*
	{
		std::stringstream ss;
		ss << l_i512H3;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	boost::multiprecision::int1024_t l_i1024U1H2 = _JacobianFloorMod(l_i1024U1 * l_i1024H2, m_P);
	/*
	{
		std::stringstream ss;
		ss << l_i512U1H2;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	boost::multiprecision::int1024_t l_i1024nx = _JacobianFloorMod(l_i1024R * l_i1024R - l_i1024H3 - 2 * l_i1024U1H2, m_P);
	/*
	{
		std::stringstream ss;
		ss << l_i512nx;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	boost::multiprecision::int1024_t l_i1024ny = _JacobianFloorMod(l_i1024R * (l_i1024U1H2 - l_i1024nx) - l_i1024S1 * l_i1024H3, m_P);
	/*
	{
		std::stringstream ss;
		ss << l_i512ny;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	boost::multiprecision::int1024_t l_i1024nz = _JacobianFloorMod(l_i1024H * l_ui1024CopyA[2] * l_ui1024CopyB[2], m_P);
	/*
	{
		std::stringstream ss;
		ss << l_i512nz;
		DebugString( (char*)ss.str().c_str() );
	}
	*/

	p_pA[0] = l_i1024nx;
	p_pA[1] = l_i1024ny;
	p_pA[2] = l_i1024nz;


	/*

		if not p[1]:
		return q
	if not q[1]:
		return p


	U1 = (p[0] * q[2] ** 2) % P
	U2 = (q[0] * p[2] ** 2) % P
	S1 = (p[1] * q[2] ** 3) % P
	S2 = (q[1] * p[2] ** 3) % P
	if U1 == U2:
		if S1 != S2:
			return (0, 0, 1)
		return jacobian_double(p)


	H = U2 - U1
	R = S2 - S1
	H2 = (H * H) % P
	H3 = (H * H2) % P
	U1H2 = (U1 * H2) % P
	nx = (R**2 - H3 - 2 * U1H2) % P
	ny = (R * (U1H2 - nx) - S1 * H3) % P
	nz = (H * p[2] * q[2]) % P
	return (nx, ny, nz)


	*/
}


void CExchgWebSocketBase::_JacobianDouble(boost::multiprecision::int1024_t* p_pA)
{
	if (0 == p_pA[1])
	{
		p_pA[0] = 0;
		p_pA[1] = 0;
		p_pA[2] = 0;
		return;
	}

	boost::multiprecision::int1024_t l_i1024Copy[3];
	l_i1024Copy[0] = p_pA[0];
	l_i1024Copy[1] = p_pA[1];
	l_i1024Copy[2] = p_pA[2];

	/*
	{
		std::stringstream ss;
		ss << l_i512Copy[0];
		DebugString( (char*)ss.str().c_str() );
	}
	{
		std::stringstream ss;
		ss << l_i512Copy[1];
		DebugString( (char*)ss.str().c_str() );
	}
	{
		std::stringstream ss;
		ss << l_i512Copy[2];
		DebugString( (char*)ss.str().c_str() );
	}
	*/


	// boost::multiprecision::int512_t l_i512YSP = _JacobianFloorMod( l_i512Copy[1] * l_i512Copy[1], m_P );
	boost::multiprecision::int1024_t l_i1024YSP = (l_i1024Copy[1] * l_i1024Copy[1]) % m_P;
	/*
	{
		boost::multiprecision::uint1024_t l_Print = (boost::multiprecision::uint1024_t)l_i1024YSP;
		std::stringstream ss;
		ss << "l_i512YSP: " << std::hex << l_Print;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	// boost::multiprecision::int512_t l_i512S = _JacobianFloorMod( 4 * l_i512Copy[0] * l_i512YSP, m_P );
	// boost::multiprecision::int512_t l_i512S = ( 4 * l_i512Copy[0] * l_i512YSP ) % m_P;
	// boost::multiprecision::int512_t l_i512S = ( ( l_i512Copy[0] * l_i512YSP ) << 2 ) % m_P;
	// boost::multiprecision::int512_t l_i512S = _JacobianFloorMod( l_i512Copy[0] * l_i512YSP * 4, m_P );
	// boost::multiprecision::int1024_t l_i512S = l_i512Copy[0];
	// boost::multiprecision::int512_t l_i512S = l_i512Copy[0];
	boost::multiprecision::int1024_t l_i512S = l_i1024Copy[0];
	/*
	{
		boost::multiprecision::uint1024_t l_Print = (boost::multiprecision::uint1024_t)l_i512S;
		std::stringstream ss;
		ss << "l_i512Copy[0]: " << std::hex << l_Print;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	l_i512S <<= 2;
	/*
	{
		boost::multiprecision::uint1024_t l_Print = (boost::multiprecision::uint1024_t)l_i512S;
		std::stringstream ss;
		ss << "4 * l_i512Copy[0]: " << std::hex << l_Print;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	l_i512S *= l_i1024YSP;
	/*
	{
		boost::multiprecision::uint1024_t l_Print = (boost::multiprecision::uint1024_t)l_i512S;
		std::stringstream ss;
		ss << "4 * l_i512Copy[0] * l_i512YSP: " << std::hex << l_Print;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	l_i512S %= m_P;
	/*
	{
		boost::multiprecision::uint1024_t l_Print = (boost::multiprecision::uint1024_t)l_i512S;
		std::stringstream ss;
		ss << "4 * l_i512Copy[0] * l_i512YSP  - m_P: " << std::hex << l_Print;
		DebugString( (char*)ss.str().c_str() );
	}
	{
		boost::multiprecision::uint1024_t l_Print = (boost::multiprecision::uint1024_t)l_i512S;
		std::stringstream ss;
		ss << "l_i512S: " << std::hex << l_Print;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	/*
	l_i512S *= 4;
	{
		std::stringstream ss;
		ss << l_i512S;
		DebugString( (char*)ss.str().c_str() );
	}
	l_i512S *= l_i512YSP;
	{
		std::stringstream ss;
		ss << l_i512S;
		DebugString( (char*)ss.str().c_str() );
	}
	l_i512S %= m_P;
	{
		std::stringstream ss;
		ss << l_i512S;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	boost::multiprecision::int1024_t l_i1024M = _JacobianFloorMod(l_i1024Copy[0] * l_i1024Copy[0] * 3, m_P);
	// boost::multiprecision::int512_t l_i512M = ( l_i512Copy[0] * l_i512Copy[0] * 3 ) % m_P;
	/*
	{
		boost::multiprecision::uint1024_t l_Print = (boost::multiprecision::uint1024_t)l_i1024M;
		std::stringstream ss;
		ss << "l_i512M: " << std::hex << l_Print;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	boost::multiprecision::int1024_t l_i1024nx = _JacobianFloorMod(l_i1024M * l_i1024M - (l_i512S << 1), m_P);
	// boost::multiprecision::int512_t l_i512nx = ( l_i512M * l_i512M - ( l_i512S << 1 ) ) % m_P;
	/*
	{
		std::stringstream ss;
		ss << l_i512nx;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	boost::multiprecision::int1024_t l_i1024ny = _JacobianFloorMod(l_i1024M * (l_i512S - l_i1024nx) - ((l_i1024YSP * l_i1024YSP) * 8), m_P);
	// boost::multiprecision::int512_t l_i512ny = ( l_i512M * ( l_i512S - l_i512nx ) - ( ( l_i512YSP * l_i512YSP ) * 8 ) ) % m_P;
	/*
	{
		boost::multiprecision::uint1024_t l_Print = (boost::multiprecision::uint1024_t)l_i1024ny;
		std::stringstream ss;
		ss << "l_i512ny: " << std::hex << l_Print;
		DebugString( (char*)ss.str().c_str() );
	}
	*/
	// boost::multiprecision::int512_t l_i512nz = _JacobianFloorMod( ( l_i512Copy[1] * l_i512Copy[2] ) << 1, m_P );
	boost::multiprecision::int1024_t l_i1024nz = ((l_i1024Copy[1] * l_i1024Copy[2]) << 1) % m_P;
	/*
	{
		std::stringstream ss;
		ss << l_i512nz;
		DebugString( (char*)ss.str().c_str() );
	}
	*/

	p_pA[0] = l_i1024nx;
	p_pA[1] = l_i1024ny;
	p_pA[2] = l_i1024nz;
}



void CExchgWebSocketBase::_JacobianMultiply(boost::multiprecision::int1024_t* p_pA, const boost::multiprecision::int1024_t p_B)
{
	if ((0 == p_pA[1]) || (0 == p_B))
	{
		p_pA[0] = 0;
		p_pA[1] = 0;
		p_pA[2] = 1;
		return;
	}

	if (1 == p_B)
	{
		return;
	}

	if ((p_B < 0) || (p_B >= m_N))
	{
		_JacobianMultiply(p_pA, _JacobianFloorMod(p_B, m_N));
		return;
	}

	if (0 == _JacobianFloorMod(p_B, 2))
	{
		_JacobianMultiply(p_pA, _JacobianFloorDiv(p_B, 2));
		_JacobianDouble(p_pA);
		return;
	}

	if (1 == _JacobianFloorMod(p_B, 2))
	{
		boost::multiprecision::int1024_t p_CopyA[3];
		p_CopyA[0] = p_pA[0];
		p_CopyA[1] = p_pA[1];
		p_CopyA[2] = p_pA[2];

		_JacobianMultiply(p_pA, _JacobianFloorDiv(p_B, 2));
		_JacobianDouble(p_pA);
		_JacobianAdd(p_pA, p_CopyA);
		return;
	}

	_ASSERT(false);
}





void CExchgWebSocketBase::_JacobianFastMultiply(boost::multiprecision::int256_t* p_pA, const boost::multiprecision::int256_t& p_B)
{
	boost::multiprecision::int1024_t l_arrayWorkingCopyA[3];
	l_arrayWorkingCopyA[0] = p_pA[0];
	l_arrayWorkingCopyA[1] = p_pA[1];
	l_arrayWorkingCopyA[2] = p_pA[2];

	boost::multiprecision::int1024_t l_arrayWorkingCopyB = p_B;

	_JacobianMultiply(l_arrayWorkingCopyA, l_arrayWorkingCopyB);
	_JacobianFrom(l_arrayWorkingCopyA);

	p_pA[0] = (boost::multiprecision::int256_t)l_arrayWorkingCopyA[0];
	p_pA[1] = (boost::multiprecision::int256_t)l_arrayWorkingCopyA[1];
	p_pA[2] = (boost::multiprecision::int256_t)l_arrayWorkingCopyA[2];
}

