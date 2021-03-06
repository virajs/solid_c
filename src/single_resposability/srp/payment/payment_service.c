#include "payment_service.h"
#include "../repository/time_sheet_repository.h"
#include "../repository/employee_repository.h"
#include <stdlib.h>

void sum_time_sheet(gpointer data, gpointer user_data) {
    struct time_record const * const time_record = time_record_converter->to_time_record(data);
    double * const result = (double * const) user_data;

    *result += (time_record->worked_time >= 7.9 && time_record->worked_time <= 8.1) ? 8 : time_record->worked_time;
}

double regular_hours(int const employee_id, int const year, month const month) {
    time_sheet_repository* time_sheet_repository = new_time_sheet_repository();
    time_record_list list = time_sheet_repository->list(employee_id, year, month);
    free_time_sheet_repository(time_sheet_repository);

    double result = 0.;
    g_list_foreach(list, &sum_time_sheet, &result);
    g_list_free(list);

    return result;
}

struct payment_calculation_result calculate_pay(int const employee_id, int const year, month const month) {
    struct payment_calculation_result result = {
        .error_code = 0,
        .error_message = "",
        .payment_result = 0.
    };

    employee_repository* employee_repository = new_employee_repository();
    if (!employee_repository) {
        result.error_code = -1;
        result.error_message = "Invalid employee repository";
        return result;
    }

    employee *employee = employee_repository->find_by_id(employee_id);
    if (!employee) {
        result.error_code = -2;
        result.error_message = "Employee not found";
        return result;
    }

    double hours = regular_hours(employee_id, year, month);
    result.payment_result = hours * (employee->salary / (40 * 4));

    free_employee(employee);
    free_employee_repository(employee_repository);

    return result;
}

struct payment_service * _new_payment_service() {
    struct payment_service *payment_service = (struct payment_service *) calloc(1, sizeof(struct payment_service));
    (*payment_service) = (struct payment_service) {
        .calculate_pay = &calculate_pay,
    };

    return payment_service;
}

void _free_payment_service(struct payment_service* employee_service) {
    free(employee_service);
}

struct payment_service* (*new_payment_service)(void) = &_new_payment_service;
void (*free_payment_service)(struct payment_service*) = &_free_payment_service;
