#include "judge_B.lib/judge.hpp"
#include "judge_B.lib/log.hpp"
#include "judge_B.lib/position.hpp"
#include<iostream>
#include<vector>
#include<time.h>
#include "judge_B.lib/reactive.hpp"

double judge::electricity_score(){
	double score = 0;

	for (vehicle v : vehicles){
		score += v.cap;
	}

	for (node n : g.nodes){
		score += n.cap;
	}

	score -= Gamma * sum_out_charge;
	return score;
}


void judge::init_input(FILE* fp){//writting after wrong cas1
	T_now = 0;
	num_orders = 0;

	std::fscanf(fp, "%zu", &N_sol);
	INFO("N_sol : %zu", N_sol);

	// Init_input , Order , Amount of Charging in the scenario
	std::fscanf(fp, "%zu %zu", &N, &M);
	INFO("%zu vertices and %zu edges will be loaded.", N, M);

	g = graph<edge>(N);
	u_output = std::vector<size_t>(M);
	v_output = std::vector<size_t>(M);
	c_output = std::vector<size_t>(M);

	for (size_t i = 0;i < M;i++){
		size_t u, v, c;//u,v are 1-index.
		std::fscanf(fp, "%zu %zu %zu", &u, &v, &c);//assume input as 1-index
		u_output[i] = u;
		v_output[i] = v;
		c_output[i] = c;
		u--, v--;// transform 1-index to 0-index
		g.add_edge(u, v, c);
		g.add_edge(v, u, c);

	}
	g.sort_edges();
	INFO("finish loading graph data");

	std::fscanf(fp, "%zu", &Daytype);

	std::fscanf(fp, "%zu %zu %zu %lf %zu", &N_div, &N_pattern, &sigma_ele, &p_event, &Delta_event);
	INFO("There are %zu intervals.", N_div);
	INFO("Pattern %zu", N_pattern);
	INFO("Noise variance : %zu", sigma_ele);
	INFO("Electricity will vary %zu.", Delta_event);

	INFO("load %zu pattern for electricity state.", N_pattern);
	pw = std::vector<std::vector< int > >(N_pattern, std::vector<int>(N_div, -1));
	for (size_t i = 0;i < N_pattern;i++){
		for (size_t j = 0;j < N_div;j++){
			int pww;
			std::fscanf(fp, "%d", &pww);
			pw[i][j] = pww;
		}
	}

	std::fscanf(fp, "%zu %zu %zu %d", &N_nano, &C_Init_Grid, &C_Max_Grid, &V_Max_Grid);
	INFO("There are %zu nano grids.", N_nano);
	INFO("Initial charge amount is %zu", C_Init_Grid);
	INFO("Maximum charge amount is %zu", C_Max_Grid);
	INFO("Maximum charge and discharge speed are %d", V_Max_Grid);
	x_output = std::vector<size_t>(N_nano);
	type_output = std::vector<size_t>(N_nano);
	for (size_t i = 0;i < N_nano;i++){
		size_t x, type;
		std::fscanf(fp, "%zu %zu", &x, &type);
		x_output[i] = x;
		type_output[i] = type;
		// 1-index to 0-index 
		INFO("A nano grid at point %zu has the electricity pattern %zu.", x - 1, type);
		g.set_node(x - 1, node(C_Max_Grid, C_Init_Grid, type, x - 1));
	}

	std::fscanf(fp, "%zu %zu %zu %d %zu %zu", &N_vehicle, &C_Init_EV, &C_Max_EV, &V_Max_EV, &N_Max_Trans, &Delta_move);
	INFO("There are %zu EVs.", N_vehicle);
	INFO("Initial charge amount is %zu", C_Init_EV);
	INFO("Maximum charge amount is %zu", C_Max_EV);
	INFO("Maximum charge and discharge speed are %d", V_Max_EV);
	INFO("Each EVs can transport %zu items at once.", N_Max_Trans);
	INFO("Each EVs consume %zu electricity for each move.", Delta_move);

	vehicles = std::vector<vehicle>(N_vehicle);
	pos_output = std::vector<size_t>(N_vehicle);
	for (size_t i = 0;i < N_vehicle;i++){
		size_t pos;
		std::fscanf(fp, "%zu", &pos);
		// 1-index to 0-index
		INFO("EV %zu is at point %zu.", i, pos - 1);
		pos_output[i] = pos;
		position now_pos = position(pos - 1, g[pos - 1][0].dst, 0, g[pos - 1][0].cost);
		vehicles[i] = vehicle(C_Max_EV, C_Init_EV, now_pos);
	}
	std::fscanf(fp, "%lf %zu", &p_trans_const, &T_last);
	INFO("A transportation request will occur at each time and at each point with a probability of %f.", p_trans_const);
	INFO("All of transportation requests will ocur until time %zu.", T_last);
	std::fscanf(fp, "%zu %lf %d %d", &P_trans, &Gamma, &S_ele_ref, &S_trans_ref);
	std::fscanf(fp, "%zu", &T_max);
	INFO("You will act %zu times.", T_max);


	orders = std::vector< std::vector<order> >((T_last + 1) * N_sol);
	energies = std::vector< std::vector<energy> >(T_max * N_sol, std::vector<energy>(N_nano));

	for (size_t k = 0;k < N_sol;k++){
		//Order

		for (size_t i = 0;i <= T_last;i++){
			size_t N_new_order;
			std::fscanf(fp, "%zu", &N_new_order);
			orders[i + k * (T_last + 1)] = std::vector<order>(N_new_order);
			for (size_t j = 0;j < N_new_order;j++){
				size_t order_id, from, to, time;
				//order_id , from,to  , time  1-index 
				std::fscanf(fp, "%zu %zu %zu %zu", &order_id, &from, &to, &time);
				order_id--, from--, to--;//1-index to 0-index

				orders[i + k * (T_last + 1)][j] = order(order_id, time, from, to);
			}
		}


		//Amount of Charging
		for (size_t i = 0;i < T_max;i++){
			for (size_t j = 0;j < N_nano;j++){
				size_t nano_id; //nano_id 1-index
				int delta;
				std::fscanf(fp, "%zu %d", &nano_id, &delta);
				nano_id--;
				energies[i + k * T_max][j] = energy(nano_id, delta);
				if (k == 0 and i == 0 and j == 0){
					INFO("nano id : %zu", nano_id);
					INFO("delta   : %d", delta);
				}
			}
		}
	}

}

void judge::copy_sol(judge& J, int sol){
	orders = std::vector< std::vector<order> >((T_last + 1));
	energies = std::vector< std::vector<energy> >(T_max);
	for (size_t i = 0; i <= T_last; i++) orders[i] = J.orders[i + (T_last + 1) * sol];
	for (size_t i = 0; i < T_max; i++) energies[i] = J.energies[i + T_max * sol];
}

bool judge::valid_input(){//contestant's input
	static char command[256];
	const size_t N_vehicles = vehicles.size();

	for (size_t vehicle_id = 0; vehicle_id < N_vehicles; ++vehicle_id){
		INFO("vehicle_id : %zu", vehicle_id);
		fscanf(stdin, "%255s", command);

		INFO("command : %s", command);

		if (strcmp(command, "stay") == 0){

		}
		else if (strcmp(command, "move") == 0){
			int dest;
			if (!fscanf(stdin, "%d", &dest)) exit(1);
			--dest;
			if (!valid_move(vehicle_id, dest)) exit(1);
		}
		else if (strcmp(command, "pickup") == 0){
			int ord_id;
			if (!fscanf(stdin, "%d", &ord_id)) exit(1);
			--ord_id;
			if (!valid_pickup(ord_id, vehicle_id)) exit(1);
		}
		else if (strcmp(command, "charge_from_grid") == 0){
			int charge;
			if (!fscanf(stdin, "%d", &charge)) exit(1);
			if (!valid_charge(vehicle_id, charge)) exit(1);
		}
		else if (strcmp(command, "charge_to_grid") == 0){
			int charge;
			if (!fscanf(stdin, "%d", &charge)) exit(1);
			if (!valid_discharge(vehicle_id, charge)) exit(1);
		}
		else{
			exit(1);
		}
	}

	return true;
}


bool judge::next_time_step(){
	if (T_now < orders.size()){
		num_orders += orders[T_now].size();
		for (size_t i = 0;i < orders[T_now].size();i++){
			now_orders.push_back(orders[T_now][i]);
		}
	}

	for (size_t i = 0;i < N_nano;i++){//Charging
		info_day.push(energies[T_now][i]);
	}

	T_now++;
	if (T_now > T_last) return false;
	return true;
}


void judge::print_all(FILE* fp){
	INFO("output \"REST ENERGY ABOUT GRID\" for contestant");
	print_rest_energy(fp);

	INFO("output \"State ABOUT Vehicle\" for contestant");
	print_vehicle(fp);

	INFO("output \"Rest Order State ABOUT Order\" for contestant");
	print_Order(fp);

	INFO("Sum of out of energies: %lu", sum_out_charge);
	fflush(fp);
}

void judge::print_vehicle(FILE* fp){
	for (size_t i = 0;i < N_vehicle;i++){
		int u, v;
		size_t N_can_go, dist_u, dist_v;//u,v 0-index
		u = vehicles[i].now_vertex();
		std::vector<size_t> can_go_vertices;
		if (u == -1){
			u = vehicles[i].pos.from;
			v = vehicles[i].pos.to;
			dist_u = vehicles[i].pos.distance;
			dist_v = vehicles[i].pos.rest_distance();
			N_can_go = 2;
			can_go_vertices.push_back(u);
			can_go_vertices.push_back(v);
		}
		else{
			v = u;
			dist_u = 0;
			dist_v = 0;
			N_can_go = 0;
			for (edge e : g.g[u]){
				N_can_go++;
				can_go_vertices.push_back(e.dst);
			}

		}
		fprintf(fp, "%d\n", vehicles[i].cap);
		fprintf(fp, "%d %d %zu %zu\n", u + 1, v + 1, dist_u, dist_v);
		fprintf(fp, "%zu", N_can_go);
		for (size_t j = 0;j < N_can_go;j++){
			fprintf(fp, " %zu", can_go_vertices[j] + 1);
		}
		fprintf(fp, "\n");
		fprintf(fp, "%zu", vehicles[i].orders.size());
		for (size_t j : vehicles[i].orders){
			fprintf(fp, " %zu", j + 1);
		}
		fprintf(fp, "\n");
	}
	return;
}


void judge::print_Initial_Output(FILE* fp){
	fprintf(fp, "%zu\n", N_sol);
	fprintf(fp, "%zu %zu\n", N, M);
	for (size_t i = 0;i < M;i++){
		fprintf(fp, "%zu %zu %zu\n", u_output[i], v_output[i], c_output[i]);
	}
	fprintf(fp, "%zu\n", Daytype);
	fprintf(fp, "%zu %zu %zu %0.1lf %zu\n", N_div, N_pattern, sigma_ele, p_event, Delta_event);
	for (size_t i = 0;i < N_pattern;i++){
		for (size_t j = 0;j < N_div;j++){
			if (j != 0) fprintf(fp, " ");
			fprintf(fp, "%d", pw[i][j]);
		}
		fprintf(fp, "\n");
	}

	fprintf(fp, "%zu %zu %zu %d\n", N_nano, C_Init_Grid, C_Max_Grid, V_Max_Grid);
	for (size_t i = 0;i < N_nano;i++){
		fprintf(fp, "%zu %zu\n", x_output[i], type_output[i]);
	}
	fprintf(fp, "%zu %zu %zu %d %zu %zu\n", N_vehicle, C_Init_EV, C_Max_EV, V_Max_EV, N_Max_Trans, Delta_move);
	for (size_t i = 0;i < N_vehicle;i++){
		fprintf(fp, "%zu\n", pos_output[i]);
	}

	fprintf(fp, "%0.2lf %zu\n", p_trans_const, T_last);
	fprintf(fp, "%zu %0.2lf %d %d\n", P_trans, Gamma, S_ele_ref, S_trans_ref);
	fprintf(fp, "%ld\n", T_max);
}


double score(std::vector<std::pair<double, double> >& Ans){
	std::sort(Ans.begin(), Ans.end());
	std::vector<std::pair<double, double> > a;
	a.push_back(Ans[0]);
	for (size_t i = 1;i < Ans.size();i++){
		bool flag = true;
		for (size_t j = i + 1;j < Ans.size();j++){
			if (Ans[i].second <= Ans[j].second){
				flag = false;
				break;
			}
		}
		if (flag){
			a.push_back(Ans[i]);
		}
	}

	double res = 0;
	for (size_t i = 1;i < a.size();i++){
		res += (a[i].first - a[i - 1].first) * (a[i].second - a[0].second);
	}
	return res;
}




double main2(){
	size_t N_sol = 0;
	std::vector<std::pair<double, double> > Answers;
	std::pair<double, double> base;


	// init_input
	judge Common;

	FILE* fp, * contestant_input, * time_step_result;
	fp = stdin;


	contestant_input = stdout;
	time_step_result = stdout;



	INFO("Start judge initialize");
	Common.init_input(fp);
	reactive_connect();
	INFO("Complete judge initialize");
	INFO("Output for contestant:");
	Common.print_Initial_Output(contestant_input);
	base = std::pair<double, double>(Common.S_trans_ref, Common.S_ele_ref);
	Answers.push_back(base);



	while (1){
		judge Judge = Common;
		Judge.g = Common.g;
		Judge.copy_sol(Common, N_sol);
		INFO("Solution %zu start.", N_sol);


		for (size_t t = 0;t < Judge.T_max;t++){
			INFO("Turn %zu", t);
			INFO("Number of Orders is %zu.", Judge.num_orders);
			Judge.next_time_step();

			Judge.print_all(time_step_result);
			if (Judge.valid_input()){
			}
			else{
				exit(1);
			}

			if (Judge.valid_end_time()){
			}
			else{
				exit(1);
			}
		}

		INFO("Successful!");

		double ts = Judge.transport_score();
		double es = Judge.electricity_score();
		INFO("TransportScore: %lf, Electricity: %lf", ts, es);

		Answers.push_back(std::pair<double, double>(std::max((double)Judge.S_trans_ref, ts), std::max((double)Judge.S_ele_ref, es)));

		INFO("Turn %zu", Judge.T_max);
		Judge.print_all(time_step_result);
		fprintf(time_step_result, "%0.2f %0.2f\n", ts, es);
		fflush(time_step_result);
		N_sol++;

		if (Judge.N_sol <= N_sol) break;
	}
	sleep(1);
	reactive_disconnect();

	return score(Answers);
}

int main(int argc, const char** argv){
	DEBUG("start reactive contest");
	reactive_start(argc, argv);
	DEBUG("start main2");
	double ans = main2();
	DEBUG("finish main2");
	printf("%0.2lf\n", ans);
	INFO("Score %0.2lf\n", ans);
	guard.is_accepted = true;
	reactive_end();
	return 0;
}
