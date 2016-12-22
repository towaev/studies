INSERT INTO CLIENTS (CLIENT_ID, DATA) VALUES (01, 'Антонов Егор');
INSERT INTO CLIENTS (CLIENT_ID, DATA) VALUES (02, 'Букреев Борис');
INSERT INTO CLIENTS (CLIENT_ID, DATA) VALUES (03, 'Воргулева Анна');
INSERT INTO CLIENTS (CLIENT_ID, DATA) VALUES (04, 'Никромов Илья');
INSERT INTO CLIENTS (CLIENT_ID, DATA) VALUES (05, 'Астахов Фарид');
COMMIT;

INSERT INTO MEDIUMS (MEDIUM_ID, “TYPE”) VALUES (1,’Рекламный щит’);
INSERT INTO MEDIUMS (MEDIUM_ID, “TYPE”) VALUES (2,’Реклама на авто’);
INSERT INTO MEDIUMS (MEDIUM_ID, “TYPE”) VALUES (3,’Реклама в интернете’);
INSERT INTO MEDIUMS (MEDIUM_ID, “TYPE”) VALUES (4,’Реклама в газете’);


INSERT INTO TIMETABLE (TIME_ID, MEDIUM_ID, INITIAL_DATE, FINAL_DATE, PRICE) 
VALUES (1, 1, ’01-10-2015’, ’01-12-2015’, 11300.0);
INSERT INTO TIMETABLE (TIME_ID, MEDIUM_ID, INITIAL_DATE, FINAL_DATE, PRICE)
VALUES (2, 1, ’02-01-2016’, ’02-02-2016’, 6000.0);
INSERT INTO TIMETABLE (TIME_ID, MEDIUM_ID, INITIAL_DATE, FINAL_DATE, PRICE)
VALUES (3, 3, ’14-11-2015’, ’14-12-2015’, 1000.0);
INSERT INTO TIMETABLE (TIME_ID, MEDIUM_ID, INITIAL_DATE, FINAL_DATE, PRICE)
VALUES (4, 3, ’20-12-2015’, ’10-01-2016’, 1500.0);
INSERT INTO TIMETABLE (TIME_ID, MEDIUM_ID, INITIAL_DATE, FINAL_DATE, PRICE)
VALUES (5, 4, ’01-02-2016’, ’15-02-2016’, 900.0);
COMMIT;

INSERT INTO CONTRACTS (CONTRACT_ID, CLIENT_ID, PRICE, TIME_ID)
VALUES (1, 2, 3000, 3);
INSERT INTO CONTRACTS (CONTRACT_ID, CLIENT_ID, PRICE, TIME_ID)
VALUES (2, 1, 2500, 1);
INSERT INTO CONTRACTS (CONTRACT_ID, CLIENT_ID, PRICE, TIME_ID)
VALUES (3, 3, 5000, 4);
INSERT INTO CONTRACTS (CONTRACT_ID, CLIENT_ID, PRICE, TIME_ID)
VALUES (4, 1, 1000, 1); 

COMMIT;

INSERT INTO SERVICES (SERVICE_ID, “TYPE”, PRICE, QUANTITY)
VALUES (1, 'Листовки', 10, 1000);
INSERT INTO SERVICES (SERVICE_ID, “TYPE”, PRICE, QUANTITY)
VALUES (2, 'Рекламный плакат', 2500, 5);
INSERT INTO SERVICES (SERVICE_ID, “TYPE”, PRICE, QUANTITY)
VALUES (3, 'Плакаты', 1000, 10);
INSERT INTO SERVICES (SERVICE_ID, “TYPE”, PRICE, QUANTITY)
VALUES (4, 'Интернет-реклама', 500, 5);
INSERT INTO SERVICES (SERVICE_ID, “TYPE”, PRICE, QUANTITY)
VALUES (5, 'ТВ-реклама', 10000, 1);
COMMIT;

INSERT INTO CONTRACTSERVICES (CO_SERV_ID, CONTRACT_ID, SERVICE_ID)
VALUES (1, 1, 3);
INSERT INTO CONTRACTSERVICES (CO_SERV_ID, CONTRACT_ID, SERVICE_ID)
VALUES (2, 1, 2);
INSERT INTO CONTRACTSERVICES (CO_SERV_ID, CONTRACT_ID, SERVICE_ID)
VALUES (3, 2, 1);
INSERT INTO CONTRACTSERVICES (CO_SERV_ID, CONTRACT_ID, SERVICE_ID)
VALUES (4, 4, 1);
INSERT INTO CONTRACTSERVICES (CO_SERV_ID, CONTRACT_ID, SERVICE_ID)
VALUES (5, 3, 5);
COMMIT;

INSERT INTO SHIPMENT (SHIPMENT_ID, “TYPE”, PRICE)
VALUES (1, 'Промоутеры', 1000);
INSERT INTO SHIPMENT (SHIPMENT_ID, “TYPE”, PRICE)
VALUES (2, 'Веб дизайнеры', 100);
INSERT INTO SHIPMENT (SHIPMENT_ID, “TYPE”, PRICE)
VALUES (3, 'Установщики щитов', 100);
INSERT INTO SHIPMENT (SHIPMENT_ID, “TYPE”, PRICE)
VALUES (4, 'ТВ-студия', 5000);
INSERT INTO SHIPMENT (SHIPMENT_ID, “TYPE”, PRICE)
VALUES (5, 'Расклейщики плакатов', 100);

COMMIT;

INSERT INTO CONTRACTSHIPMENTS (CO_SHIP_ID, CO_SERV_ID, SHIPMENT_ID)
VALUES (1, 2, 3);
INSERT INTO CONTRACTSHIPMENTS (CO_SHIP_ID, CO_SERV_ID, SHIPMENT_ID)
VALUES (1, 4, 1);
INSERT INTO CONTRACTSHIPMENTS (CO_SHIP_ID, CO_SERV_ID, SHIPMENT_ID)
VALUES (1, 1, 1);
INSERT INTO CONTRACTSHIPMENTS (CO_SHIP_ID, CO_SERV_ID, SHIPMENT_ID)
VALUES (1, 5, 5);
INSERT INTO CONTRACTSHIPMENTS (CO_SHIP_ID, CO_SERV_ID, SHIPMENT_ID)
VALUES (1, 3, 3);
COMMIT;
