#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>

enum{
  READ, WRITE
};

int to_contestant[2], from_contestant[2];
int stdio[2];

bool connected_to_contestant = false;

void init(){
  if (pipe(to_contestant)) return exit(1);
  if (pipe(from_contestant)) return exit(1);
  stdio[READ] = dup(0); // stdin
  stdio[WRITE] = dup(1); // stdout
}

void connect_to_contestant(){
  if (connected_to_contestant) return;
  fflush(stdout);
  dup2(from_contestant[READ], 0);
  dup2(to_contestant[WRITE], 1);
  connected_to_contestant = true;
}

void disconnect_from_contestant(){
  if (!connected_to_contestant) return;
  fflush(stdout);
  dup2(stdio[READ], 0);
  dup2(stdio[WRITE], 1);

  connected_to_contestant = false;
}

int main(int argc, const char** argv){
  using namespace std;

  {
    ofstream dest("contest.log");

    dest << "# start contest\n";
    dest << "# conststant : \"" << argv[1] << "\"\n";
    dest << "# prepare pipes for communication: ";
    init();
    dest << "finish" << endl;
    dest.close();
  }

  pid_t child_pid = fork();
  if (child_pid < 0) return 1;

  if (child_pid == 0){
    // child proccess
    // replace to ocntestant proccess

    // this memory will be released by OS
    const char** argv_new = (const char**)malloc(sizeof(char*) * argc);
    for (size_t i = 0; i + 1 < argc; ++i)
      argv_new[i] = argv[i + 1];
    argv_new[argc - 1] = nullptr;

    close(from_contestant[READ]);
    close(to_contestant[WRITE]);

    dup2(from_contestant[WRITE], 1);
    dup2(to_contestant[READ], 0);

    close(from_contestant[WRITE]);
    close(to_contestant[READ]);

    cerr << "# exec contestant proccess\n";
    execv(argv_new[0], (char* const*)argv_new);
    cerr << "ERROR!\n";
    return 1;
  }
  close(from_contestant[WRITE]);
  close(to_contestant[READ]);
  cerr << "# child proccess: " << child_pid << "\n";
  ofstream dest("contest.log", ios::app);

  dest << "# IO format 1\n";

  string line;
  size_t N_solution;
  cin >> N_solution;
  dest << "# N_solution\n";
  dest << N_solution << "\n";
  size_t V, E;
  cin >> V >> E;
  dest << "# |V| |E|\n";
  dest << V << " " << E << "\n";
  vector<size_t> u(E), v(E), d(E);
  dest << "# u v d\n";
  for (size_t i = 0; i < E; ++i){
    cin >> u[i] >> v[i] >> d[i];
    dest << u[i] << " " << v[i] << " " << d[i] << "\n";
  }
  size_t DayType;
  cin >> DayType;
  dest << "# DayType\n";
  dest << DayType << "\n";
  size_t N_div, N_pattern, sigma_2_ele;
  double p_event;
  size_t Delta_event;
  cin >> N_div >> N_pattern >> sigma_2_ele >> p_event >> Delta_event;
  dest << "# N_div N_pattern sigma^2_ele p_event Delta_event\n";
  dest << N_div << " " << N_pattern << " " << sigma_2_ele << " " << p_event << " " << Delta_event << "\n";
  dest << "# pw\n";
  vector<vector<int>> pw(N_pattern, vector<int>(N_div));
  for (auto& pw_line : pw){
    cin >> pw_line[0];
    dest << pw_line[0];
    for (size_t i = 1; i < N_div; ++i){
      cin >> pw_line[i];
      dest << " " << pw_line[i];
    }
    dest << "\n";
  }
  size_t N_grid, C_grid_init, C_grid_max, V_grid_max;
  cin >> N_grid >> C_grid_init >> C_grid_max >> V_grid_max;
  dest << "# N_grid C_grid_init C_grid_max V_grid_max\n";
  dest << N_grid << " " << C_grid_init << " " << C_grid_max << " " << V_grid_max << "\n";
  vector<size_t> x(N_grid), pattern(N_grid);
  dest << "# x pattern\n";
  for (size_t i = 0; i < N_grid; ++i){
    cin >> x[i] >> pattern[i];
    dest << x[i] << " " << pattern[i] << "\n";
  }
  size_t N_EV, C_EV_init, C_EV_max, V_EV_max, N_trans_max, Delta_EV_move;
  cin >> N_EV >> C_EV_init >> C_EV_max >> V_EV_max >> N_trans_max >> Delta_EV_move;
  dest << "# N_EV C_EV_init C_EV_max V_EV_max N_trans_max Delta_EV_move\n";
  dest << N_EV << " " << C_EV_init << " " << C_EV_max << " " << V_EV_max << " " << N_trans_max << " " << Delta_EV_move << "\n";
  vector<size_t> pos(N_EV);
  dest << "# pos\n";
  for (size_t i = 0; i < N_EV; ++i){
    cin >> pos[i];
    dest << pos[i] << "\n";
  }
  double p_const_trans;
  size_t T_last;
  cin >> p_const_trans >> T_last;
  dest << "# p_const_trans T_last\n";
  dest << p_const_trans << " " << T_last << "\n";
  size_t P_trans;
  double gamma;
  int S_ref_ele, S_ref_trans;
  cin >> P_trans >> gamma >> S_ref_ele >> S_ref_trans;
  dest << "# P_trans gamma S_ref_ele S_ref_trans\n";
  dest << P_trans << " " << gamma << " " << S_ref_ele << " " << S_ref_trans << "\n";
  size_t T_max;
  cin >> T_max;
  dest << "# T_max\n";
  dest << T_max << "\n";
  dest << flush;

  connect_to_contestant();
  cout << N_solution << "\n";
  cout << V << " " << E << "\n";
  for (size_t i = 0; i < E; ++i) cout << u[i] << " " << v[i] << " " << d[i] << "\n";
  cout << DayType << "\n";
  cout << N_div << " " << N_pattern << " " << sigma_2_ele << " " << p_event << " " << Delta_event << "\n";
  for (auto& pw_line : pw){
    cout << pw_line[0];
    for (size_t i = 1; i < N_div; ++i) cout << " " << pw_line[i];
    cout << "\n";
  }
  cout << N_grid << " " << C_grid_init << " " << C_grid_max << " " << V_grid_max << "\n";
  for (size_t i = 0; i < N_grid; ++i)
    cout << x[i] << " " << pattern[i] << "\n";
  cout << N_EV << " " << C_EV_init << " " << C_EV_max << " " << V_EV_max << " " << N_trans_max << " " << Delta_EV_move << "\n";
  for (size_t i = 0; i < N_EV; ++i)
    cout << pos[i] << "\n";
  cout << p_const_trans << " " << T_last << "\n";
  cout << P_trans << " " << gamma << " " << S_ref_ele << " " << S_ref_trans << "\n";
  cout << T_max << endl;
  disconnect_from_contestant();

  vector<size_t> y(N_grid);
  vector<int> pw_actual(N_grid);
  vector<size_t> pw_excess(N_grid), pw_buy(N_grid);
  size_t tmp;
  for (size_t n = 0; n < N_solution; ++n){
    cerr << "# start challenge: " << n + 1 << "\n";
    dest << "# start challenge: " << n + 1 << "\n";
    dest << "# IO format 2\n";
    for (size_t t = 0; t <= T_max; ++t){
      dest << "# turn: " << t << "\n";
      dest << "# x y pw_actual pw_excess pw_buy\n";
      for (size_t i = 0; i < N_grid; ++i){
        cin >> tmp >> y[i] >> pw_actual[i] >> pw_excess[i] >> pw_buy[i];
        dest << x[i] << " " << y[i] << " " << pw_actual[i] << " " << pw_excess[i] << " " << pw_buy[i] << "\n";
      }

      connect_to_contestant();
      for (size_t i = 0; i < N_grid; ++i)
        cout << x[i] << " " << y[i] << " " << pw_actual[i] << " " << pw_excess[i] << " " << pw_buy[i] << "\n";
      disconnect_from_contestant();

      dest << "# charge\n";
      dest << "# u v dist_from_u dist_to_v\n";
      dest << "# N_adj a ...\n";
      dest << "# N_order o ...\n";
      for (size_t i = 0; i < N_EV; ++i){
        dest << "# EV " << i + 1 << "\n";
        size_t charge, u, v, dist_from_u, dist_to_v, N_adj, N_order;
        vector<size_t> a, o;
        cin >> charge;
        dest << charge << "\n";
        cin >> u >> v >> dist_from_u >> dist_to_v;
        dest << u << " " << v << " " << dist_from_u << " " << dist_to_v << "\n";
        cin >> N_adj; a.resize(N_adj); dest << N_adj;
        for (size_t j = 0; j < N_adj; ++j) cin >> a[j], dest << " " << a[j];
        dest << "\n";
        cin >> N_order; o.resize(N_order); dest << N_order;
        for (size_t j = 0; j < N_order; ++j) cin >> o[j], dest << " " << o[j];
        dest << "\n";
        connect_to_contestant();
        cout << charge << "\n";
        cout << u << " " << v << " " << dist_from_u << " " << dist_to_v << "\n";
        cout << N_adj;
        for (size_t j = 0; j < N_adj; ++j) cout << " " << a[j];
        cout << "\n";
        cout << N_order;
        for (size_t j = 0; j < N_order; ++j) cout << " " << o[j];
        cout << "\n";
        disconnect_from_contestant();
      }
      size_t N_order;
      cin >> N_order;
      dest << "# N_order\n";
      dest << N_order << "\n";
      vector<size_t> id(N_order), w(N_order), z(N_order), state(N_order), time(N_order);
      dest << "# id w z state order\n";
      for (size_t j = 0; j < N_order; ++j){
        cin >> id[j] >> w[j] >> z[j] >> state[j] >> time[j];
        dest << id[j] << " " << w[j] << " " << z[j] << " " << state[j] << " " << time[j] << "\n";
      }
      connect_to_contestant();
      cout << N_order << "\n";
      for (size_t j = 0; j < N_order; ++j)
        cout << id[j] << " " << w[j] << " " << z[j] << " " << state[j] << " " << time[j] << "\n";
      if (t == T_max) break;
      vector<string> command(N_EV);
      dest << "# command\n";
      for (size_t j = 0; j < N_EV; ++j){
        do{
          getline(cin, command[j]);
        } while (command[j] == "");
        dest << "EV_" << j + 1 << ": \"" << command[j] << "\"\n";
      }
      disconnect_from_contestant();
      for (size_t j = 0; j < N_EV; ++j)
        cout << command[j] << "\n";
      cout << flush;
    }
    double S_trans, S_ele;
    cin >> S_trans >> S_ele;
    dest << "# S_trans S_ele\n";
    dest << S_trans << " " << S_ele << "\n";

    connect_to_contestant();
    cout << S_trans << " " << S_ele << "\n";
    disconnect_from_contestant();
    dest << flush;
  }
  dest << "# all finished\n";
  return 0;
}