#include "Blackboard.hpp"
#include <fstream>

void BlackboardData::modifyData(const std::string& key, const std::any& value) {
	if (data.find(key) != data.end()) {
		try {
			data[key] = value;
		}
		catch (const std::bad_any_cast&) {
			if (verbose)
				std::cerr << "TypeMismatch" << std::endl;
			throw TypeMismatch{};
		}
	}
	else if (parentData) {
		parentData->modifyData(key, value);
	}
	else {
		if (verbose)
			std::cerr << "KeyNotFound : " << key << std::endl;
		throw KeyNotFound{};
	}
}


std::map<std::string, std::any> BlackboardData::getAllData() const {
	if (parentData) {
		std::map<std::string, std::any> parentMap = parentData->getAllData();
		for (auto& entry : data) {			
				parentMap.insert(entry);
		}
		return parentMap;
	}
	else {
		return data;
	}
}



std::string BlackboardData::ecrireBlackboardDansFichier(const std::string& nomFichier, int nbparent) {
	std::string s{};
	for (const auto& entry : data) {
		for (size_t i = 0; i < nbparent; i++)
		{
			s += "\t";
		}
		s += entry.first + "\n";

	}
	if (parentData) {
		std::string s1 = parentData->ecrireBlackboardDansFichier(nomFichier, nbparent + 1);
		s.append(s1);
	}

	if (nbparent == 0)
	{
		std::ofstream fichier(nomFichier); // Crée un fichier pour l'écriture

		fichier << s;

	}
	return s;
}