#include "acl_stdafx.hpp"
#include "acl_cpp/stdlib/string.hpp"
#include "acl_cpp/stdlib/dbuf_pool.hpp"
#include "acl_cpp/stdlib/snprintf.hpp"
#include "acl_cpp/redis/redis_client.hpp"
#include "acl_cpp/redis/redis_result.hpp"
#include "acl_cpp/redis/redis_list.hpp"

namespace acl
{

#define INT_LEN		11
#define LONG_LEN	21

redis_list::redis_list(redis_client* conn /* = NULL */)
: conn_(conn)
, result_(NULL)
{

}

redis_list::~redis_list()
{

}

void redis_list::reset()
{
	if (conn_)
		conn_->reset();
}

void redis_list::set_client(redis_client* conn)
{
	conn_ = conn;
}

//////////////////////////////////////////////////////////////////////////

int redis_list::llen(const char* key)
{
	const char* argv[2];
	size_t lens[2];

	argv[0] = "LLEN";
	lens[0] = sizeof("LLEN") - 1;
	argv[1] = key;
	lens[1] = strlen(key);

	const string& req = conn_->build_request(2, argv, lens);
	result_ = conn_->run(req);
	if (result_ == NULL)
		return -1;
	if (result_->get_type() != REDIS_RESULT_INTEGER)
		return -1;
	return result_->get_integer();
}

bool redis_list::lindex(const char* key, size_t idx, string& buf,
	bool* exist/*  = NULL */)
{
	if (exist)
		*exist = false;

	const char* argv[3];
	size_t lens[3];

	argv[0] = "LINDEX";
	lens[0] = sizeof("LINDEX") - 1;
	argv[1] = key;
	lens[1] = strlen(key);

	char tmp[LONG_LEN];
	(void) safe_snprintf(tmp, sizeof(tmp), "%lu", (unsigned long) idx);
	argv[2] = tmp;
	lens[2] = strlen(tmp);

	const string& req = conn_->build_request(3, argv, lens);
	result_ = conn_->run(req);
	if (result_ == NULL)
		return false;
	if (result_->get_type() != REDIS_RESULT_STRING)
		return false;
	if (result_->argv_to_string(buf) > 0 && exist != NULL)
		*exist = true;
	return true;
}

bool redis_list::lset(const char* key, size_t idx, const char* value)
{
	return lset(key, idx, value, strlen(value));
}

bool redis_list::lset(const char* key, size_t idx,
	const char* value, size_t len)
{
	const char* argv[4];
	size_t lens[4];

	argv[0] = "LSET";
	lens[0] = sizeof("LSET") - 1;
	argv[1] = key;
	lens[1] = strlen(key);

	char tmp[LONG_LEN];
	(void) safe_snprintf(tmp, sizeof(tmp), "%lu", (unsigned long) idx);
	argv[2] = tmp;
	lens[2] = strlen(tmp);

	argv[3] = value;
	lens[3] = len;

	const string& req = conn_->build_request(4, argv, lens);
	result_ = conn_->run(req);
	if (result_ == NULL)
		return false;
	if (result_->get_type() != REDIS_RESULT_STATUS)
		return false;
	const char* res = result_->get_status();
	if (res == NULL || strcasecmp(res, "OK") != 0)
		return false;
	else
		return true;
}

int redis_list::linsert_before(const char* key, const char* pivot,
	const char* value)
{
	return linsert_before(key, pivot, strlen(pivot), value, strlen(value));
}

int redis_list::linsert_before(const char* key, const char* pivot,
	size_t pivot_len, const char* value, size_t value_len)
{
	return linsert(key, "BEFORE", pivot, pivot_len, value, value_len);
}

int redis_list::linsert_after(const char* key, const char* pivot,
	const char* value)
{
	return linsert_after(key, pivot, strlen(pivot), value, strlen(value));
}

int redis_list::linsert_after(const char* key, const char* pivot,
	size_t pivot_len, const char* value, size_t value_len)
{
	return linsert(key, "AFTER", pivot, pivot_len, value, value_len);
}

int redis_list::linsert(const char* key, const char* pos, const char* pivot,
	size_t pivot_len, const char* value, size_t value_len)
{
	const char* argv[5];
	size_t lens[5];

	argv[0] = "LINSERT";
	lens[0] = sizeof("LINSERT") - 1;
	argv[1] = key;
	lens[1] = strlen(key);
	argv[2] = pos;
	lens[2] = strlen(pos);
	argv[3] = pivot;
	lens[3] = pivot_len;
	argv[4] = value;
	lens[4] = value_len;

	const string& req = conn_->build_request(5, argv, lens);
	result_ = conn_->run(req);
	if (result_ == NULL || result_->get_type() != REDIS_RESULT_INTEGER)
		return -1;
	return result_->get_integer();
}

int redis_list::lpush(const char* key, const char* first_value, ...)
{
	std::vector<const char*> values;
	values.push_back(first_value);

	va_list ap;
	va_start(ap, first_value);
	const char* value;

	while ((value = va_arg(ap, const char*)) != NULL)
		values.push_back(value);
	va_end(ap);

	return lpush(key, values);
}

int redis_list::lpush(const char* key, const char* values[], size_t argc)
{
	const string& req = conn_->build("LPUSH", key, values, argc);
	return push(req);
}

int redis_list::lpush(const char* key, const std::vector<string>& values)
{
	const string& req = conn_->build("LPUSH", key, values);
	return push(req);
}

int redis_list::lpush(const char* key, const std::vector<char*>& values)
{
	const string& req = conn_->build("LPUSH", key, values);
	return push(req);
}

int redis_list::lpush(const char* key, const std::vector<const char*>& values)
{
	const string& req = conn_->build("LPUSH", key, values);
	return push(req);
}

int redis_list::lpush(const char* key, const char* values[],
	size_t lens[], size_t argc)
{
	const string& req = conn_->build("LPUSH", key, values, lens, argc);
	return push(req);
}

int redis_list::rpush(const char* key, const char* first_value, ...)
{
	std::vector<const char*> values;
	values.push_back(first_value);

	va_list ap;
	va_start(ap, first_value);
	const char* value;

	while ((value = va_arg(ap, const char*)) != NULL)
		values.push_back(value);
	va_end(ap);

	return rpush(key, values);
}

int redis_list::rpush(const char* key, const char* values[], size_t argc)
{
	const string& req = conn_->build("RPUSH", key, values, argc);
	return push(req);
}

int redis_list::rpush(const char* key, const std::vector<string>& values)
{
	const string& req = conn_->build("RPUSH", key, values);
	return push(req);
}

int redis_list::rpush(const char* key, const std::vector<char*>& values)
{
	const string& req = conn_->build("RPUSH", key, values);
	return push(req);
}

int redis_list::rpush(const char* key, const std::vector<const char*>& values)
{
	const string& req = conn_->build("RPUSH", key, values);
	return push(req);
}

int redis_list::rpush(const char* key, const char* values[],
	size_t lens[], size_t argc)
{
	const string& req = conn_->build("RPUSH", key, values, lens, argc);
	return push(req);
}

int redis_list::push(const string& req)
{
	result_ = conn_->run(req);
	if (result_ == NULL)
		return -1;
	if (result_->get_type() != REDIS_RESULT_INTEGER)
		return -1;
	return result_->get_integer();
}

int redis_list::lpushx(const char* key, const char* value)
{
	return lpushx(key, value, strlen(value));
}

int redis_list::lpushx(const char* key, const char* value, size_t len)
{
	return pushx("LPUSHX", key, value, len);
}

int redis_list::rpushx(const char* key, const char* value)
{
	return rpushx(key, value, strlen(value));
}

int redis_list::rpushx(const char* key, const char* value, size_t len)
{
	return pushx("RPUSHX", key, value, len);
}

int redis_list::pushx(const char* cmd, const char* key,
	const char* value, size_t len)
{
	const char* argv[3];
	size_t lens[3];

	argv[0] = cmd;
	lens[0] = strlen(cmd);
	argv[1] = key;
	lens[1] = strlen(key);
	argv[2] = value;
	lens[2] = len;

	const string& req = conn_->build_request(3, argv, lens);
	result_ = conn_->run(req);
	if (result_ == NULL)
		return -1;
	if (result_->get_type() != REDIS_RESULT_INTEGER)
		return -1;
	return result_->get_integer();
}

//////////////////////////////////////////////////////////////////////////

int redis_list::lpop(const char* key, string& buf)
{
	return pop("LPOP", key, buf);
}

int redis_list::rpop(const char* key, string& buf)
{
	return pop("RPOP", key, buf);
}

int redis_list::pop(const char* cmd, const char* key, string& buf)
{
	const char* argv[2];
	size_t lens[2];

	argv[0] = cmd;
	lens[0] = strlen(cmd);
	argv[1] = key;
	lens[1] = strlen(key);

	const string& req = conn_->build_request(2, argv, lens);
	result_ = conn_->run(req);
	if (result_ == NULL)
		return -1;
	if (result_->get_type() != REDIS_RESULT_STRING)
		return -1;

	return result_->argv_to_string(buf);
}

bool redis_list::blpop(std::pair<string, string>& result, size_t timeout,
	const char* first_key, ...)
{
	std::vector<const char*> keys;
	keys.push_back(first_key);

	va_list ap;
	va_start(ap, first_key);
	const char* key;
	while ((key = va_arg(ap, const char*)) != NULL)
		keys.push_back(key);
	va_end(ap);
	return blpop(keys, timeout, result);
}

bool redis_list::blpop(const std::vector<char*>& keys, size_t timeout,
	std::pair<string, string>& result)
{
	return blpop((const std::vector<const char*>&) keys, timeout, result);
}

bool redis_list::blpop(const std::vector<const char*>& keys, size_t timeout,
	std::pair<string, string>& result)
{
	return bpop("BLPOP", keys, timeout, result);
}


bool redis_list::blpop(const std::vector<string>& keys, size_t timeout,
	std::pair<string, string>& result)
{
	return bpop("BLPOP", keys, timeout, result);
}

bool redis_list::brpop(std::pair<string, string>& result, size_t timeout,
	const char* first_key, ...)
{
	std::vector<const char*> keys;
	keys.push_back(first_key);

	va_list ap;
	va_start(ap, first_key);
	const char* key;
	while ((key = va_arg(ap, const char*)) != NULL)
		keys.push_back(key);
	va_end(ap);
	return brpop(keys, timeout, result);
}

bool redis_list::brpop(const std::vector<char*>& keys, size_t timeout,
	std::pair<string, string>& result)
{
	return brpop((const std::vector<const char*>&) keys, timeout, result);
}

bool redis_list::brpop(const std::vector<const char*>& keys, size_t timeout,
	std::pair<string, string>& result)
{
	return bpop("BRPOP", keys, timeout, result);
}


bool redis_list::brpop(const std::vector<string>& keys, size_t timeout,
	std::pair<string, string>& result)
{
	return bpop("BRPOP", keys, timeout, result);
}

bool redis_list::bpop(const char* cmd, const std::vector<const char*>& keys,
	size_t timeout, std::pair<string, string>& result)
{
	size_t argc = 2 + keys.size();
	dbuf_pool* pool = conn_->get_pool();
	const char** args = (const char**)
		pool->dbuf_alloc(argc * sizeof(char*));
	size_t* lens = (size_t*) pool->dbuf_alloc(argc * sizeof(size_t));

	args[0] = cmd;
	lens[0] = strlen(cmd);

	size_t i = 1;
	std::vector<const char*>::const_iterator cit = keys.begin();
	for (; cit != keys.end(); ++cit)
	{
		args[i] = *cit;
		lens[i] = strlen(args[i]);
		i++;
	}

	char buf[LONG_LEN];
	safe_snprintf(buf, sizeof(buf), "%lu", (unsigned long) timeout);
	args[i] = buf;
	lens[i] = strlen(args[i]);

	const string& req = conn_->build_request(argc, args, lens);
	return bpop(req, result);
}

bool redis_list::bpop(const char* cmd, const std::vector<string>& keys,
	size_t timeout, std::pair<string, string>& result)
{
	size_t argc = 2 + keys.size();
	dbuf_pool* pool = conn_->get_pool();
	const char** args = (const char**)
		pool->dbuf_alloc(argc * sizeof(char*));
	size_t* lens = (size_t*) pool->dbuf_alloc(argc * sizeof(size_t));

	args[0] = cmd;
	lens[0] = strlen(cmd);

	size_t i = 1;
	std::vector<string>::const_iterator cit = keys.begin();
	for (; cit != keys.end(); ++cit)
	{
		args[i] = (*cit).c_str();
		lens[i] = strlen(args[i]);
		i++;
	}

	char buf[LONG_LEN];
	safe_snprintf(buf, sizeof(buf), "%lu", (unsigned long) timeout);
	args[i] = buf;
	lens[i] = strlen(args[i]);

	const string& req = conn_->build_request(argc, args, lens);
	return bpop(req, result);
}

bool redis_list::bpop(const string& req, std::pair<string, string>& result)
{
	result_ = conn_->run(req);
	if (result_ == NULL)
		return false;
	if (result_->get_type() != REDIS_RESULT_ARRAY)
		return false;
	size_t size = result_->get_size();
	if (size == 0)
		return false;
	if (size != 2)
		return false;

	const redis_result* first = result_->get_child(0);
	const redis_result* second = result_->get_child(1);
	if (first == NULL || second == NULL
		|| first->get_type() != REDIS_RESULT_STRING
		|| second->get_type() != REDIS_RESULT_STRING)
	{
		return false;
	}

	string buf;
	first->argv_to_string(buf);
	result.first = buf;

	buf.clear();
	second->argv_to_string(buf);
	result.second = buf;
	return true;
}

bool redis_list::rpoplpush(const char* src, const char* dst,
	string* buf /* = NULL */)
{
	const char* argv[3];
	size_t lens[3];

	argv[0] = "RPOPLPUSH";
	lens[0] = sizeof("RPOPLPUSH") - 1;
	argv[1] = src;
	lens[1] = strlen(src);
	argv[2] = dst;
	lens[2] = strlen(dst);

	const string& req = conn_->build_request(3, argv, lens);
	result_ = conn_->run(req);
	if (result_ == NULL || result_->get_type() != REDIS_RESULT_STRING)
		return false;
	if (buf == NULL)
		return true;
	result_->argv_to_string(*buf);
	return true;
}

bool redis_list::brpoplpush(const char* src, const char* dst,
	size_t timeout, string* buf /* = NULL */)
{
	const char* argv[4];
	size_t lens[4];

	argv[0] = "BRPOPLPUSH";
	lens[0] = sizeof("BRPOPLPUSH") - 1;
	argv[1] = src;
	lens[1] = strlen(src);
	argv[2] = dst;
	lens[2] = strlen(dst);

	char tmp[LONG_LEN];
	safe_snprintf(tmp, sizeof(tmp), "%lu", (unsigned long) timeout);
	argv[3] = tmp;
	lens[3] = strlen(argv[3]);

	const string& req = conn_->build_request(4, argv, lens);
	result_ = conn_->run(req);
	if (result_ == NULL || result_->get_type() != REDIS_RESULT_STRING)
		return false;
	if (buf == NULL)
		return true;
	result_->argv_to_string(*buf);
	return true;
}

bool redis_list::lrange(const char* key, size_t start, size_t end,
	std::vector<string>& result)
{
	const char* argv[4];
	size_t lens[4];

	argv[0] = "LRANGE";
	lens[0] = sizeof("LRANGE") - 1;
	argv[1] = key;
	lens[1] = strlen(key);

	char start_s[LONG_LEN], end_s[LONG_LEN];
	safe_snprintf(start_s, sizeof(start_s), "%lu", (unsigned long) start);
	safe_snprintf(end_s, sizeof(end_s), "%lu", (unsigned long) end);

	argv[2] = start_s;
	lens[2] = strlen(start_s);
	argv[3] = end_s;
	lens[3] = strlen(end_s);

	const string& req = conn_->build_request(4, argv, lens);
	result_ = conn_->run(req);
	if (result_ == NULL || result_->get_type() != REDIS_RESULT_ARRAY)
		return false;

	size_t size;
	const redis_result** children = result_->get_children(&size);
	if (children == NULL)
		return true;

	string buf;
	for (size_t i = 0; i < size; i++)
	{
		const redis_result* rr = children[i];
		rr->argv_to_string(buf);
		result.push_back(buf);
		buf.clear();
	}

	return true;
}

int redis_list::lrem(const char* key, int count, const char* value)
{
	return lrem(key, count, value, strlen(value));
}

int redis_list::lrem(const char* key, int count, const char* value, size_t len)
{
	const char* argv[4];
	size_t lens[4];

	argv[0] = "LREM";
	lens[0] = sizeof("LREM") - 1;
	argv[1] = key;
	lens[1] = strlen(key);

	char buf[INT_LEN];
	safe_snprintf(buf, sizeof(buf), "%d", count);
	argv[2] = buf;
	lens[2] = strlen(buf);

	argv[3] = value;
	lens[3] = len;

	const string& req = conn_->build_request(4, argv, lens);
	result_ = conn_->run(req);
	if (result_ == NULL || result_->get_type() != REDIS_RESULT_INTEGER)
		return -1;
	return result_->get_integer();
}

bool redis_list::ltrim(const char* key, size_t start, size_t end)
{
	const char* argv[4];
	size_t lens[4];

	argv[0] = "LTRIM";
	lens[0] = sizeof("LTRIM") - 1;
	argv[1] = key;
	lens[1] = strlen(key);

	char start_s[LONG_LEN], end_s[LONG_LEN];
	safe_snprintf(start_s, sizeof(start_s), "%lu", (unsigned long) start);
	safe_snprintf(end_s, sizeof(end_s), "%lu", (unsigned long) end);

	argv[2] = start_s;
	lens[2] = strlen(start_s);
	argv[3] = end_s;
	lens[3] = strlen(end_s);

	const string& req = conn_->build_request(4, argv, lens);
	result_ = conn_->run(req);
	if (result_ == NULL || result_->get_type() != REDIS_RESULT_STATUS)
		return false;
	const char* status = result_->get_status();
	if (status == NULL || strcasecmp(status, "OK") != 0)
		return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////

} // namespace acl
