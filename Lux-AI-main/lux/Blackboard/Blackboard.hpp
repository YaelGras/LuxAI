#ifndef BLACKBOARD
#define BLACKBOARD

#include <memory>
#include <map>
#include <string>
#include <any>
#include <iostream>
class TypeMismatch {};
class KeyNotFound {};

class BlackboardData {
	std::shared_ptr<BlackboardData> parentData;
	std::map<std::string, std::any> data;
	bool verbose = false;

public:
	BlackboardData() = default;
	BlackboardData(bool _verbose) : verbose(_verbose) {}
	BlackboardData(std::shared_ptr<BlackboardData> _parent)
		: parentData(_parent) {
		verbose = parentData->isVerbose();
	}
	BlackboardData(std::shared_ptr<BlackboardData> _parent, bool _verbose) 
		: verbose(_verbose), parentData( _parent ) {}
	~BlackboardData() = default;

	bool isVerbose() const {
		return verbose;
	}
	void setVerbose(bool _verbose) {
		verbose = _verbose;
	}

	void insertData(const std::string& key, const std::any& value) {
		data[key] = value;
	}

	void modifyData(const std::string& key, const std::any& value);

	template <typename T>
	T& getData(const std::string& key) {
		if (data.find(key) != data.end()) {
			try {
				return std::any_cast<T&>(data[key]);
			}
			catch (const std::bad_any_cast&) {
				if(verbose)
					std::cerr << "TypeMismatch"<< std::endl;
				throw TypeMismatch{};
			}
		}
		else if (parentData){
			return parentData->getData<T>(key);
		}
		else {
			if(verbose)
			std::cerr << "KeyNotFound : " << key << std::endl;
			throw KeyNotFound{};
		}
	}

	std::map<std::string, std::any> getAllData() const;

	std::string ecrireBlackboardDansFichier(const std::string& nomFichier, int nbparent);

};


#endif 