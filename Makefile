marketplace: anikin-taxi-http.o marketplace.o marketplace2.o marketplace2_json.o anikin-taxi-http.o marketplace_api.o geo.o geo_core.o geo_core_best_match.o http_interface.o ws_interface.o console_interface.o fifo_interface.o auth.o ws_interface_test.o ws_interface_external.o marketplace_api_db.o http_interface_auth.o http_interface_geo.o http_interface_rider.o http_interface_driver.o marketplace_assigned_order.o geolocation.o marketplace_api_drivers.o marketplace_api_riders.o http_interface_internal.o marketplace_dumper.o auth_dumper.o object_track.o motion_simulator.o polylineencoder.o driver_billing.o trie.o cost_db.o marketplace2_rebalance.o profiler.o

marketplace:
	gcc -O3 -rpath /usr/local/mysql-connector-c++-8.0.21/lib64 -lh3 -L /usr/local/mysql-connector-c++-8.0.21/lib64 -I /usr/local/mysql-connector-c++-8.0.21/include/ -lmysqlcppconn8.2.8.0.21 -lstdc++ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -o anikin-taxi-http marketplace.o marketplace2.o marketplace2_json.o anikin-taxi-http.o marketplace_api.o geo.o geo_core.o geo_core_best_match.o http_interface.o ws_interface.o console_interface.o fifo_interface.o auth.o ws_interface_test.o ws_interface_external.o marketplace_api_db.o http_interface_auth.o http_interface_geo.o http_interface_rider.o http_interface_driver.o marketplace_assigned_order.o geolocation.o marketplace_api_drivers.o marketplace_api_riders.o http_interface_internal.o marketplace_dumper.o auth_dumper.o object_track.o motion_simulator.o polylineencoder.o driver_billing.o trie.o cost_db.o marketplace2_rebalance.o profiler.o

test_motion_simulator:
	gcc -O3 -rpath /usr/local/mysql-connector-c++-8.0.21/lib64 -lh3 -L /usr/local/mysql-connector-c++-8.0.21/lib64 -I /usr/local/mysql-connector-c++-8.0.21/include/ -lmysqlcppconn8.2.8.0.21 -lstdc++ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -o test_motion_simulator test_motion_simulator.cpp motion_simulator.o geolocation.o polylineencoder.o

test_motion_simulator: test_motion_simulator.cpp geolocation.o polylineencoder.o motion_simulator.o

anikin-taxi-auth:
	gcc -O3 -rpath /usr/local/mysql-connector-c++-8.0.21/lib64 -lh3 -L /usr/local/mysql-connector-c++-8.0.21/lib64 -I /usr/local/mysql-connector-c++-8.0.21/include/ -lmysqlcppconn8.2.8.0.21 -lstdc++ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -o anikin-taxi-auth anikin-taxi-auth.cpp

anikin-taxi-auth:	anikin-taxi-auth.cpp

osm_parser:
	gcc -O3 -rpath /usr/local/mysql-connector-c++-8.0.21/lib64 -lh3 -L /usr/local/mysql-connector-c++-8.0.21/lib64 -I /usr/local/mysql-connector-c++-8.0.21/include/ -lmysqlcppconn8.2.8.0.21 -lstdc++ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -o osm_parser osm_parser.cpp

osm_parser:	osm_parser.cpp


test_riders_drivers: test_riders_drivers.cpp
	gcc -O3 -rpath /usr/local/mysql-connector-c++-8.0.21/lib64 -lh3 -L /usr/local/mysql-connector-c++-8.0.21/lib64 -I /usr/local/mysql-connector-c++-8.0.21/include/ -lmysqlcppconn8.2.8.0.21 -lstdc++ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -o test_riders_drivers test_riders_drivers.cpp

polylineencoder.o:	polylineencoder.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o polylineencoder.o polylineencoder.cpp

profiler.o:	profiler.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o profiler.o profiler.cpp


motion_simulator.o:	motion_simulator.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o motion_simulator.o motion_simulator.cpp

anikin-taxi-http.o: anikin-taxi-http.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o anikin-taxi-http.o anikin-taxi-http.cpp

marketplace_api.o: marketplace_api.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o marketplace_api.o marketplace_api.cpp

geo.o: geo.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o geo.o geo.cpp

geo_core.o: geo_core.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o geo_core.o geo_core.cpp

geo_core_best_match.o: geo_core_best_match.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o geo_core_best_match.o geo_core_best_match.cpp

http_interface.o: http_interface.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o http_interface.o http_interface.cpp

ws_interface.o: ws_interface.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o ws_interface.o ws_interface.cpp

console_interface.o: console_interface.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o console_interface.o console_interface.cpp

marketplace.o: marketplace.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o marketplace.o marketplace.cpp

marketplace2.o: marketplace2.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o marketplace2.o marketplace2.cpp

marketplace2_json.o: marketplace2_json.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o marketplace2_json.o marketplace2_json.cpp


fifo_interface.o: fifo_interface.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o fifo_interface.o fifo_interface.cpp

auth.o: auth.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o auth.o auth.cpp

ws_interface_test.o: ws_interface_test.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o ws_interface_test.o ws_interface_test.cpp

ws_interface_external.o: ws_interface_external.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o ws_interface_external.o ws_interface_external.cpp

marketplace_api_db.o: marketplace_api_db.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o marketplace_api_db.o marketplace_api_db.cpp

http_interface_auth.o: http_interface_auth.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o http_interface_auth.o http_interface_auth.cpp

http_interface_geo.o: http_interface_geo.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o http_interface_geo.o http_interface_geo.cpp

http_interface_rider.o: http_interface_rider.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o http_interface_rider.o http_interface_rider.cpp

http_interface_driver.o: http_interface_driver.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o http_interface_driver.o http_interface_driver.cpp

marketplace_assigned_order.o: marketplace_assigned_order.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o marketplace_assigned_order.o marketplace_assigned_order.cpp


geolocation.o: geolocation.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o geolocation.o geolocation.cpp


marketplace_api_drivers.o: marketplace_api_drivers.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o marketplace_api_drivers.o marketplace_api_drivers.cpp


marketplace_api_riders.o: marketplace_api_riders.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o marketplace_api_riders.o marketplace_api_riders.cpp



http_interface_internal.o: http_interface_internal.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o http_interface_internal.o http_interface_internal.cpp

marketplace_dumper.o: marketplace_dumper.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o marketplace_dumper.o marketplace_dumper.cpp


auth_dumper.o: auth_dumper.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o auth_dumper.o auth_dumper.cpp


object_track.o: object_track.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o object_track.o object_track.cpp

driver_billing.o: driver_billing.cpp
	gcc -O3 -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o driver_billing.o driver_billing.cpp

trie.o: trie.cpp
	gcc -O3 -c -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -o trie.o trie.cpp

cost_db.o: cost_db.cpp
	gcc -O3 -c -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -o cost_db.o cost_db.cpp

marketplace2_rebalance.o: marketplace2_rebalance.cpp
	gcc -O3 -c -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -o marketplace2_rebalance.o marketplace2_rebalance.cpp


trie_test.o: trie_test.cpp
	gcc -O3 -c -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -o trie_test.o trie_test.cpp


trie_test: trie.o trie_test.o
	gcc -O3 -lstdc++ -I /usr/local/mysql-connector-c++-8.0.21/include/ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -o trie_test trie.o trie_test.o

balanced_diet.o: balanced_diet.cpp
	 gcc -O3 -std=gnu++11 -Wreserved-user-defined-literal -Wc++11-long-long -Wc++11-extensions -c -o balanced_diet.o balanced_diet.cpp	

balanced_diet_suggest.o: balanced_diet_suggest.cpp
	 gcc -O3 -std=gnu++11 -Wreserved-user-defined-literal -Wc++11-long-long -Wc++11-extensions -c -o balanced_diet_suggest.o balanced_diet_suggest.cpp	

balanced_diet_data.o: balanced_diet_data.cpp
	 gcc -O3 -std=gnu++11 -Wreserved-user-defined-literal -Wc++11-long-long -Wc++11-extensions -c -o balanced_diet_data.o balanced_diet_data.cpp	

balanced_diet_recipe.o: balanced_diet_recipe.cpp
	 gcc -O3 -std=gnu++11 -Wreserved-user-defined-literal -Wc++11-long-long -Wc++11-extensions -c -o balanced_diet_recipe.o balanced_diet_recipe.cpp	

balanced_diet_food_rank.o: balanced_diet_food_rank.cpp
	 gcc -O3 -std=gnu++11 -Wreserved-user-defined-literal -Wc++11-long-long -Wc++11-extensions -c -o balanced_diet_food_rank.o balanced_diet_food_rank.cpp	

balanced_diet_auto_balance.o: balanced_diet_auto_balance.cpp
	 gcc -O3 -std=gnu++11 -Wreserved-user-defined-literal -Wc++11-long-long -Wc++11-extensions -c -o balanced_diet_auto_balance.o balanced_diet_auto_balance.cpp	

balanced_diet_search.o: balanced_diet_search.cpp
	 gcc -O3 -std=gnu++11 -Wreserved-user-defined-literal -Wc++11-long-long -Wc++11-extensions -c -o balanced_diet_search.o balanced_diet_search.cpp	

balanced_diet_upload.o: balanced_diet_upload.cpp
	 gcc -O3 -std=gnu++11 -Wreserved-user-defined-literal -Wc++11-long-long -Wc++11-extensions -c -o balanced_diet_upload.o balanced_diet_upload.cpp	

balanced_diet_stat.o: balanced_diet_stat.cpp
	 gcc -O3 -std=gnu++11 -Wreserved-user-defined-literal -Wc++11-long-long -Wc++11-extensions -c -o balanced_diet_stat.o balanced_diet_stat.cpp	



balanced_diet_test.o: balanced_diet_test.cpp
	 gcc -O3 -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -c -o balanced_diet_test.o balanced_diet_test.cpp	

balanced_diet_test: balanced_diet_test.o balanced_diet.o balanced_diet_suggest.o balanced_diet_data.o balanced_diet_recipe.o balanced_diet_food_rank.o balanced_diet_auto_balance.o balanced_diet_search.o balanced_diet_upload.o balanced_diet_stat.o
	gcc -O3 -lstdc++ -std=gnu++11 -Wc++11-long-long -Wc++11-extensions -o balanced_diet_test balanced_diet.o balanced_diet_test.o balanced_diet_suggest.o balanced_diet_data.o balanced_diet_recipe.o balanced_diet_food_rank.o balanced_diet_auto_balance.o balanced_diet_search.o balanced_diet_upload.o balanced_diet_stat.o

