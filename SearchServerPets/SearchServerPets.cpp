#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
	string s;
	getline(cin, s);
	return s;
}

int ReadLineWithNumber() {
	int result;
	cin >> result;
	ReadLine();
	return result;
}

vector<string> SplitIntoWords(const string& text) {
	vector<string> words;
	string word;
	for (const char c : text) {
		if (c == ' ') {
			words.push_back(word);
			word = "";
		}
		else {
			word += c;
		}
	}
	words.push_back(word);

	return words;
}

struct Document {
	int id;
	double relevance;
};
struct Query {
	vector<string> plus;
	vector<string> minus;
};

class SearchServer {
public:
	void SetStopWords(const string& text) {
		for (const string& word : SplitIntoWords(text)) {
			stop_words_.insert(word);
		}
	}

	void AddDocument(int document_id, const string& document) {
		vector<string> v = SplitIntoWordsNoStop(document);
		++document_count;
		for (const string& word : v) {
			word_to_document_freqs_[word].insert({ document_id, (count(v.begin(), v.end(), word) * 1.0) / (v.size() * 1.0) });
		}
	}


	vector<Document> FindTopDocuments(const string& query) const {
		auto matched_documents = FindAllDocuments(query);

		sort(
			matched_documents.begin(),
			matched_documents.end(),
			[](const Document& lhs, const Document& rhs) {
				return lhs.relevance > rhs.relevance;
			}
		);
		if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
			matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
		}
		return matched_documents;
	}
private:
	map<string, map<int, double>> word_to_document_freqs_;
	set<string> stop_words_;
	int document_count = 0;

	vector<string> SplitIntoWordsNoStop(const string& text) const {
		vector<string> words;
		for (const string& word : SplitIntoWords(text)) {
			if (stop_words_.count(word) == 0) {
				words.push_back(word);
			}
		}
		return words;
	}

	vector<Document> FindAllDocuments(const string& query) const {
		const Query query_words = ParseQuery(query);
		map<int, double> document_to_relevance;

		for (const string& word : query_words.plus) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			for (const auto [document_id, tfidf] : word_to_document_freqs_.at(word)) {
				document_to_relevance[document_id] += tfidf * (log(document_count / (word_to_document_freqs_.at(word).size() * 1.0)));
			}
		}
		for (const string& word : query_words.minus) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			for (const auto [document_id, tfidf] : word_to_document_freqs_.at(word)) {
				document_to_relevance.erase(document_id);
			}
		}


		vector<Document> matched_documents;
		for (auto [document_id, relevance] : document_to_relevance) {
			matched_documents.push_back({ document_id, relevance });
		}

		return matched_documents;
	}

	Query ParseQuery(const string& query) const {
		Query query_words;
		vector<string> v = SplitIntoWordsNoStop(query);
		for (int i = 0, sz = v.size(); i < sz; i++) {
			if (v[i][0] == '-') {
				query_words.minus.push_back(v[i].substr(1));
			}
			else query_words.plus.push_back(v[i]);
		}

		return query_words;
	}
};
SearchServer CreateSearchServer() {
	SearchServer search_server;
	search_server.SetStopWords(ReadLine());
	const int document_count = ReadLineWithNumber();
	for (int document_id = 0; document_id < document_count; ++document_id) {
		search_server.AddDocument(document_id, ReadLine());
	}

	return search_server;
}




int main() {
	const SearchServer search_server = CreateSearchServer();

	const string query = ReadLine();
	for (auto [document_id, relevance] : search_server.FindTopDocuments(query)) {
		cout << "{ document_id = " << document_id << ", relevance = " << relevance << " }" << endl;
	}
	return 0;
}