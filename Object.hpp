#ifndef EXP_H
#define EXP_H

#include <stdint.h>
#include <string>
#include <vector>
#include <stdexcept>

typedef uint32_t Hash;
static constexpr size_t HASHED_SIZE = sizeof(Hash);

struct Object;

struct InnerObject{
	virtual Hash getHash() = 0;
	virtual std::string toString() = 0;
	virtual Object clone() = 0;
	virtual ~InnerObject(){};

	static Hash combineHash(Hash left, Hash right){
		static constexpr Hash a = 0x7483fe02, b = 0x6473281a, c = 0xab4872f0;
		return a * left + b * right + c;//TODO: get a better hash?
	}
};


struct Object{
	InnerObject* obj;
	bool this_owns_obj;
	Object(InnerObject* inner, bool transfer_ownership)
			:obj(inner), this_owns_obj(transfer_ownership){
		if(transfer_ownership)
			printf("Object c'tor recived ownership of inner object %p\n", inner);
	}
	Object(Object&& other) noexcept
			:obj(other.obj), this_owns_obj(other.this_owns_obj){
		other.this_owns_obj = false;
		printf("Object passed ownership of inner object at: %p\n", obj);
	}
	void operator=(Object& other){
		if(this_owns_obj){
			delete obj;
		}
		obj = other.obj;
		this_owns_obj = false;
	}
	void operator=(Object&& other){
		if(this_owns_obj){
			delete obj;
		}
		obj = other.obj;
		this_owns_obj = true;
	}
	~Object(){
		if(this_owns_obj){
			printf("~Object deleting inner object at: %p\n", obj);
			delete obj;
		}
	}
	Object():obj(nullptr), this_owns_obj(true){}
	Object clone(){
		return obj->clone();
	}
	Hash getHash(){
		if(obj == nullptr)
			return 0;
		return obj->getHash();
	}
	std::string toString(){
		if(obj == nullptr)
			return "None";
		return obj->toString();
	}
	bool operator<(Object other){
		return getHash() < other.getHash();
	}
};


struct Number : public InnerObject{
	int value;
	Number(int value)
		:value(value){}
	virtual Hash getHash() override{
		return value;
	}
	virtual std::string toString(){
		std::string res = std::to_string(value);;
		// printf("returning: %s\n", res.c_str());
		return res;
	}
	virtual Object clone(){
		Number* num = new Number(value);
		printf("allocated new number object at: %p\n", num);
		return Object(num, true);
	}
	virtual ~Number(){};
};

// struct Char : public InnerObject{
// 	char value;
// 	virtual Hash getHash() override{
// 		return value;
// 	}
// };

struct Collection : public InnerObject{
	struct AbstractIterator{
		virtual Object& getCurrent() = 0;
		virtual void getNext(AbstractIterator** set_it) = 0;
		virtual ~AbstractIterator(){};
	};
	struct CollectionIterator{
		CollectionIterator(AbstractIterator* it, bool transfer_ownership)
			:it(it), this_owns_it(transfer_ownership){}
		virtual ~CollectionIterator(){
			if(this_owns_it && it != nullptr){
				printf("deleting iterator object at: %p\n", it);
				delete it;
			}
		}
		AbstractIterator* it;
		bool this_owns_it;

		Object& operator*(){
			if(it == nullptr){
				throw std::runtime_error("iterator dereference was requested after end.\n");
			}
			return it->getCurrent();
		}
		void operator++(){
			AbstractIterator* next;
			it->getNext(&next);
			printf("deleting iterator object at: %p\n", it);
			delete it;
			it = next;
		}
		bool operator!=(const CollectionIterator& other){
			return it != other.it;
		}
	};
	virtual CollectionIterator begin(){
		return CollectionIterator(inner_begin(), true);
	}
	virtual CollectionIterator end(){
		return CollectionIterator(nullptr, true);
	}

	virtual Hash getHash() override{
		Hash hash = 0;
		for(Object& obj: *this){
			hash = combineHash(hash, obj.getHash());
		}
		return hash;
	}

	std::string toString(){
		std::string res = "";
		std::string next_delim = "";
		for(Object& element: *this){
			res += next_delim;
			res += element.toString();
			next_delim = ", ";
		}
		return res.substr(0, res.size()-next_delim.size());
	}

	virtual void insert_and_own(Object&& obj) = 0;
	virtual void insert(Object& obj) = 0;
	virtual void initialize_empty(Collection** out) = 0;

	virtual Object clone() override{
		Collection* res;
		initialize_empty(&res);
		for(Object& element: *this){
			res->insert_and_own(element.clone());
		}
		return Object(res, true);
	}

protected:
	virtual AbstractIterator* inner_begin() = 0;
};

struct List : public Collection{
	List():data(){}

	struct ListIterator : public AbstractIterator{
		ListIterator(int idx, List* container)
			:idx(idx), container(container){}
		virtual ~ListIterator(){};
		virtual Object& getCurrent() override{
			return container->data[idx];
		}
		virtual void getNext(AbstractIterator** set_it) override{
			if((long unsigned)(idx+1) < container->data.size()){
				*set_it = new ListIterator(idx+1, container);
				printf("allocated new iterator object at: %p\n", *set_it);
			} else {
				*set_it = nullptr;
			}
		}
		int idx;
		List* container;
	};

	std::vector<Object> data;

	virtual ListIterator* inner_begin() override{
		ListIterator* itp = new ListIterator(0, this);
		printf("allocated new iterator object at: %p\n", itp);
		return itp;
	}

	virtual void insert_and_own(Object&& obj) override{
		printf("rvalue insert was called\n");
		obj.this_owns_obj = false;
		data.push_back(Object(obj.obj, obj.this_owns_obj));
	}
	virtual void insert(Object& obj) override{
		printf("lvalue insert was called\n");
		data.push_back(Object(obj.obj, false));
	}

	virtual void initialize_empty(Collection** out){
		*out = new List();
		printf("allocated new list object at: %p\n", *out);
	}
};

// struct String : public List{
	
// };

// struct Dict : public Collection{
// 	struct DictIterator : public AbstractIterator{
// 		DictIterator(Hash key, Dict* container);
// 	};

// 	void insert(Object* key, Object* value);
// };

#endif