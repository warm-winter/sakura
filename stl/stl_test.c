#include <iostream>
#include <cstdlib>
#include <cstring>
using namespace std;
class Element {
private:
    int number;
public:
    Element() :number(0) {
        cout << "ctor" << endl;
    }
    Element(int num):number(num) {
        cout << "ctor" << endl;
    }
    Element(const Element& e):number(e.number) {
        cout << "copy ctor" << endl;
    }
    Element(Element&& e):number(e.number) {
        cout << "right value ctor" << endl;
    }
    ~Element() {
        cout << "dtor" << endl;
    }
    void operator=(const Element& item) {
        number = item.number;
    }
    bool operator==(const Element& item) {
        return (number == item.number);
    }
    void operator()() {
        cout << number ;
    }
    int GetNumber() {
        return number;
    }
};
template<typename T>
class Vector {
private:
    T* items;
    int count;
public:
    Vector() :count{ 0 }, items{nullptr} {

    }
    Vector(const Vector& vector) :count{vector.count} {
        items = static_cast<T*>(malloc(sizeof(T) * count));
        memcpy(items, vector.items, sizeof(T) * count);
    }
    Vector(Vector&& vector) :count{ vector.count }, items{ vector.items } {
        //TODO
        vector.count = 0;
        vector.items = nullptr;
    }
    ~Vector() {
        //TODO
        Clear();
        free(items);
    }
    T& operator[](int index){
        if (index<0||index>=count) {
            cout<<"invalid index"<<endl;
            return items[0];
        }
        return items[index];
    }
    int returnCount(){
        return count;
    }
    void Clear() {
        //TODO
        for(int i =0;i<count;i++){
            (items + i) -> ~T();

        }
        items = nullptr;
        count = 0;
    }
    void Add(const T& item) {

        //由于分配的时候就是有多少，分了多少，所以增加的话，必须重新申请了
        T *items_plus_one = static_cast<T*>(malloc(sizeof(T) * count+1));
        int count_plus_one = count + 1;//不能count++？
        int i;
        for(i =0;i<count;i++){
            new(items_plus_one+i) T(move(*(items+i)));
        }
        new(items_plus_one+i) T(move(item));
        //把原来的区域清空
        Clear();//clear会把count也搞没掉，，，
        count = count_plus_one;

        items = items_plus_one;
    }

    bool Insert(const T& item,int index) {
        //TODO
        if(!(index >=0 && index <=count)){

            return false;
        }else{
            if(index == count){
                Add(index);
            }else{
                T *items_plus_one = static_cast<T*>(malloc(sizeof(T) * count+1));
                int count_plus_one = count + 1;//不能count++？

                for(int i =0;i<index;i++){
                    cout<<i<<endl;
                    new(items_plus_one+i) T(move(*(items+i)));
                }
                new(items_plus_one+index) T(move(item));
                for(int i = index+1;i<=count;i++){
                    new(items_plus_one+i) T(move(*(items+i-1)));
                }

                Clear();//clear会把count也搞没掉，，，
                count = count_plus_one;
                items = items_plus_one;

            }
            return true;
        }





    }
    bool Remove(int index) {
        if(!(index>=0 && index<=count)){
            return false;
        }else{
            //TODO
            T *items_plus_one = static_cast<T*>(malloc(sizeof(T) * count-1));
            int count_plus_one = count - 1;//不能count++？

            for(int i =0;i<index;i++){
                new(items_plus_one+i) T(move(*(items+i)));
            }
            for(int i = index;i<count-1;i++){
                new(items_plus_one+i) T(move(*(items+i+1)));
            }
            Clear();//clear会把count也搞没掉，，，
            count = count_plus_one;
            items = items_plus_one;

        }return true;

    }
    int Contains(const T& item) {
        //TODO
        for(int i = 0;i<count;i++){
            if(items[i] == item){
                return i;
            }
        }
        return -1;
    }
};
template<typename T>
void PrintVector(Vector<T>& v){
    int count=v.returnCount();
    for (int i = 0; i < count; i++)
    {
        v[i]();
        cout << " ";
    }
    cout << endl;
}
int main() {
    Vector<Element>v;
    for (int i = 0; i < 4; i++) {
        Element e(i);
        v.Add(e);
    }
    PrintVector(v);
    Element e2(4);
    if (!v.Insert(e2, 10))
    {
        v.Insert(e2, 2);
    }
    PrintVector(v);
    if (!v.Remove(10))
    {
        v.Remove(2);
    }
    PrintVector(v);
    Element e3(1), e4(10);
    cout << v.Contains(e3) << endl;
    cout << v.Contains(e4) << endl;//-1 1
    Vector<Element>v2(v);
    Vector<Element>v3(move(v2));
    PrintVector(v3);
    v2.Add(e3);
    PrintVector(v2);
    return 0;
}