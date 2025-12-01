
#define  NOB_IMPLEMENTATION
#include "../nob.h"

int main(int argc, char** argv) {
	Nob_File_Paths children = {0};
	int result = 0;
	NOB_GO_REBUILD_URSELF(argc, argv);
	const char* initial_dir = nob_get_current_dir_temp();
	const char* exe = nob_temp_running_executable_path();
	const char* test_dir = nob_temp_dir_name(exe);
	nob_set_current_dir(test_dir);

	if (!nob_read_entire_dir("./cases", &children)) nob_return_defer(false);

	Nob_Cmd cmds = {0};
	Nob_Procs procs = {0};

	for (size_t i = 0; i < children.count; ++i) {
		if (strcmp(children.items[i], ".") == 0) continue;
		if (strcmp(children.items[i], "..") == 0) continue;
		Nob_String_View filepath = nob_sv_from_cstr(children.items[i]);
		if (!nob_sv_end_with(filepath, ".t")) continue;

		Nob_String_Builder src_sb = {0};
		nob_sb_append_cstr(&src_sb, "./cases/");
		nob_sb_append_cstr(&src_sb, children.items[i]);
		nob_sb_append_null(&src_sb);

		Nob_String_Builder dst_sb = {0};
		nob_sb_append_cstr(&dst_sb, "./cases/");
		nob_sb_append_cstr(&dst_sb, children.items[i]);
		nob_sb_append_cstr(&dst_sb, ".out");
		nob_sb_append_null(&dst_sb);

		Nob_String_Builder err_sb = {0};
		nob_sb_append_cstr(&err_sb, "./cases/");
		nob_sb_append_cstr(&err_sb, children.items[i]);
		nob_sb_append_cstr(&err_sb, ".err");
		nob_sb_append_null(&err_sb);

		Nob_String_Builder  expected_path_sb = {0};
		nob_sb_append_cstr(&expected_path_sb, "./output/");
		nob_sb_append_cstr(&expected_path_sb, children.items[i]);
		nob_sb_append_null(&expected_path_sb);

		nob_cmd_append(&cmds, "../main.exe", "--quiet", "--run", src_sb.items);
		if (!nob_cmd_run(&cmds, .stdout_path = dst_sb.items, .stderr_path = err_sb.items)) nob_return_defer(false);


		Nob_String_Builder expected_str = {0};
		Nob_String_Builder actual_str = {0};
		Nob_String_Builder err_str = {0};
		if (!nob_read_entire_file(expected_path_sb.items, &expected_str)) {
			nob_log(NOB_ERROR, "Expected file didn't exist: %s", expected_str.items);
			nob_return_defer(false);
		}
		if (!nob_read_entire_file(dst_sb.items, &actual_str)) {
			nob_log(NOB_ERROR, "Actual file didn't exist: %s", actual_str.items);
			nob_return_defer(false);
		}
        bool has_err = false;
		if (nob_read_entire_file(err_sb.items, &err_str)) {
            has_err = err_str.count > 0;
		}

		Nob_String_View expected = nob_sv_from_parts(expected_str.items, expected_str.count);
		Nob_String_View actual   = nob_sv_from_parts(actual_str.items, actual_str.count);
		expected = nob_sv_trim(expected);
		actual   = nob_sv_trim(actual);

		Nob_String_View expected_chop = expected;
		Nob_String_View actual_chop = actual;

		bool pass = true;
		while (actual_chop.count && expected_chop.count) {
			Nob_String_View expected_line = nob_sv_chop_by_delim(&expected_chop, '\n');
			Nob_String_View actual_line   = nob_sv_chop_by_delim(&actual_chop, '\n');

			expected_line = nob_sv_trim_right(expected_line);
			actual_line   = nob_sv_trim_right(actual_line);
			if (!nob_sv_eq(expected_line, actual_line)) {
				pass = false;
				break;
			}
		}
		pass = pass && (actual_chop.count == 0 && expected_chop.count == 0) && !has_err;

		if (pass) {
			printf("\x1b[32;1mTEST PASSED\x1b[0m '%s'\n", children.items[i]);
		} else {
			printf("\x1b[31;1mTEST FAILED\x1b[0m '%s'\n", children.items[i]);
            if (has_err) {
                printf("Stderr: \n%.*s\n", (int)err_str.count, err_str.items);
            }
			printf("Expected: \n%.*s\n", (int)expected.count, expected.data);
			printf("Actual  : \n%.*s\n", (int)actual.count, actual.data);
		}
	}

defer:
	nob_set_current_dir(initial_dir);
	return result;
}
