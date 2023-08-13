#include <iostream>
#include "Object.hpp"

using namespace std;

int main(){
	Object o = Object(new Number(30), true);
	List l;
	l.insert(o);
	l.insert_and_own(o.clone());
	
	Collection::CollectionIterator it = l.begin();
	for(;it != l.end(); ++it){
		cout << (*it).toString() << endl;
	}

	cout << l.toString() << endl;
}