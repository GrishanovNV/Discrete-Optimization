// Knapsack Problem
// 
// Стратегии Оптимизации для решения данной задачи:
// Branch and Bound
// BackTrack
// Greedy Search

#include<cstdio>
#include<vector>
#include<cassert>
#include<algorithm>
#include<tuple>
using namespace std;

struct Item
{
	int index;
	int value;
	int weight;

	// сначала берем предмет с высокой плотностью
	bool operator < (Item rhs) const
	{
		return static_cast<double>(value) / weight > static_cast<double>(rhs.value) / rhs.weight;
	}
};

// получить ожидаемое максимальное значение из текущей емкости(та что осталась) и текущего нерешенного предмета
//тут и далее ожидание = ожидаемое значение возвращаемое данной функцией
auto get_expectation(const vector<Item> & items, int capacity, int start)
{
	auto expectation = 0.0;
	for(auto i = start; i < items.size(); ++i)
	{
		auto item = items[i];
		if(capacity >= item.weight)
		{
			expectation += item.value;
			capacity -= item.weight;
		}
		// если текущей емкости недостаточно, чтобы взять предмет целиком, положим его долю в рюкзак
		// и добавим ту же долю его стоимости к ожидаемому значению
		else
		{
			expectation += static_cast<double>(item.value) * capacity / item.weight;
			break;
		}
	}

	return expectation;
}

// поиск максимального значения и предстоящий выбор:
// брать / не брать для каждого предмета
auto search(const vector<Item> & items,  int capacity)
{
	auto max_value = 0.0;
	auto max_taken = vector<int>(items.size(), 0);

	// чтобы предотвратить переполнение стека, вместо использования простой рекурсии я сам поддерживаю стек
	// элемент стека состоит из 5 частей:
	// value:         значение, накопленное на данный момент
	// capacity:      оставшаяся емкость
	// expectation:   верхняя граница значения, которое может быть получено с помощью оставшейся емкости
	// taken:         текущий выбор / запрет на выбор каждого элемента
	// pos:           следующий элемент для рассмотрения

	auto start_value = 0.0;
	auto start_capacity = capacity;
	auto start_expectation = get_expectation(items, capacity, 0);
	auto start_taken = vector<int>(items.size(), 0);
	auto start_pos = 0;

	using StackElem = tuple<double, int, double, vector<int>, int>;
	vector<StackElem> stack;
	stack.push_back(make_tuple(start_value, start_capacity, start_expectation, start_taken, start_pos));
	while(!stack.empty())
	{
		auto [cur_value, cur_capacity, cur_expectation, cur_taken, cur_pos] = stack.back();
		stack.pop_back();

		// если оставшейся емкости недостаточно, то вернемся к кандидату "откат" - соответственно методу Backtracking
		if(cur_capacity < 0) continue;
		
		// если текущее ожидание меньше наилучшего значения, то вернемся к кандидату "откат"
		if(cur_expectation <= max_value) continue;

		// если максимальное значение меньше текущего значения, обновить максимальное значение и его варианты выбора элемента
		if(max_value < cur_value)
		{
			max_value = cur_value;
			max_taken = cur_taken;
		}

		// если следующего пункта для рассмотрения не существует, то вернемся
		if(cur_pos >= items.size()) continue;

		auto cur_item = items[cur_pos];
    
		// попытаемся не брать следующий предмет
        auto notake_value = cur_value;
        auto notake_capacity = cur_capacity;
        auto notake_expectation = notake_value + get_expectation(items, notake_capacity, cur_pos + 1);
        auto notake_taken = cur_taken;
        
        stack.push_back(make_tuple(notake_value, notake_capacity, notake_expectation, notake_taken, cur_pos + 1));
    
		// попытаемся взять следующий предмет
        auto take_value = cur_value + cur_item.value;
        auto take_capacity = cur_capacity - cur_item.weight;
        auto take_expectation = take_value + get_expectation(items, take_capacity, cur_pos + 1);
        auto take_taken = cur_taken;
        take_taken[cur_item.index] = 1;
        
        stack.push_back(make_tuple(take_value, take_capacity, take_expectation, take_taken, cur_pos + 1));
	}
	return make_tuple(static_cast<int>(max_value), max_taken);
}

// печать содержимого вектора
auto print_vec(const vector<int> & vec, FILE * f = stdout)
{
	for(auto i = 0; i < vec.size(); ++i)
	{
		fprintf(f, "%d", vec[i]);
		if(i + 1 == vec.size()) fprintf(f, "\n");
		else fprintf(f, " ");
	}
}

auto load_item(const char * filename)
{
	auto f = fopen(filename, "r");
	assert(f);

	int item_count, capacity;
	fscanf(f, "%d %d", &item_count, &capacity);
	vector<Item> items;
	for(auto i = 0; i < item_count; ++i)
	{
		int value;
		int weight;
		fscanf(f, "%d %d", &value, &weight);

		items.push_back(Item{ i, value, weight });
	}

	fclose(f);

	return make_tuple(items, capacity);
}

auto save_item(const char * filename, int value, const vector<int> & taken)
{
	// записываем результат в cpp_output.txt, чтобы solver.py мог считывать его оттуда
	auto f = fopen(filename, "w");
	assert(f);

	fprintf(f, "%d 1\n", value);
	print_vec(taken, f);
	fclose(f);
}

int main(int argc, char * argv[])
{
    if(argc < 2)
    {
        printf("Usage: ./main <data-file>\n");
        printf("Example: ./main data/ks_30_0\n");
        exit(-1);
    }

	
	auto [items, capacity] = load_item(argv[1]);

	// отсортируем предметы так, чтобы они располагались в порядке убывания плотности значений
	sort(items.begin(), items.end());

	auto [value, taken] = search(items, capacity);

	printf("%d 1\n", value);
	print_vec(taken);

	save_item("cpp_output.txt", value, taken);

	return 0;
}
