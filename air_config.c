/*
  +----------------------------------------------------------------------+
  | air framework                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) wukezhan<wukezhan@gmail.com>                           |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: wukezhan<wukezhan@gmail.com>                                 |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"

#include "php_air.h"

#include "air_config.h"
#include "air_exception.h"

zend_class_entry *air_config_ce;

void air_config_merge_default(zval *data){
	zval _data;
	array_init(&_data);

	zval app;
	array_init(&app);
	add_assoc_stringl_ex(&app, ZEND_STRL("path"), ZEND_STRL("app"));

	zval exec;
	array_init(&exec);
	add_assoc_stringl_ex(&exec, ZEND_STRL("path"), ZEND_STRL("exec"));
	add_assoc_zval_ex(&app, ZEND_STRL("exec"), &exec);

	zval site;
	array_init(&site);
	add_assoc_stringl_ex(&site, ZEND_STRL("path"), ZEND_STRL("site"));
	add_assoc_zval_ex(&app, ZEND_STRL("site"), &site);

	zval view;
	array_init(&view);
	add_assoc_stringl_ex(&view, ZEND_STRL("engine"), ZEND_STRL("air\\view"));
	add_assoc_stringl_ex(&view, ZEND_STRL("path"), ZEND_STRL("view"));
	add_assoc_stringl_ex(&view, ZEND_STRL("type"), ZEND_STRL(".php"));
	add_assoc_zval_ex(&app, ZEND_STRL("view"), &view);

	add_assoc_zval_ex(&_data, ZEND_STRL("app"), &app);
	if(data != NULL && Z_TYPE_P(data) == IS_ARRAY){
		php_array_replace_recursive(Z_ARRVAL_P(&_data), Z_ARRVAL_P(data));
	}
	zend_update_static_property(air_config_ce, ZEND_STRL("_data"), &_data);
	zval_ptr_dtor(&_data);
}

zval *air_config_get_data(){
	zval *data = zend_read_static_property(air_config_ce, ZEND_STRL("_data"), 1);
	if(Z_TYPE_P(data) == IS_NULL){
		air_config_merge_default(data);
		data = zend_read_static_property(air_config_ce, ZEND_STRL("_data"), 1);
	}
	return data;
}

zval *air_config_get(zval *data, zend_string *key) {
	zval *_data = data;
	if(data == NULL){
		_data = air_config_get_data();
	}
	return zend_hash_find(Z_ARRVAL_P(_data), key);
}

zval *air_config_str_get(zval *data, char *key, int key_len) {
	zval *_data = data;
	if(data == NULL){
		_data = air_config_get_data();
	}
	return zend_hash_str_find(Z_ARRVAL_P(_data), key, key_len);
}

zval *air_config_path_get(zval *data, zend_string *path) {
	zval *_data = data;
	if(data == NULL){
		_data = air_config_get_data();
	}
	zval **tmp = NULL;
	char *seg = NULL, *sptr = NULL;
	char *skey = estrndup(ZSTR_VAL(path), ZSTR_LEN(path));
	seg = php_strtok_r(skey, ".", &sptr);
	int status = SUCCESS;
	while(seg){
		if(Z_TYPE_P(_data) != IS_ARRAY) {
			_data = NULL;
			break;
		}
		_data = zend_hash_str_find(Z_ARRVAL_P(_data), seg, strlen(seg));
		if(!_data){
			php_error(E_NOTICE, "config path '%s' not found\n", path);
			break;
		}
		seg = php_strtok_r(NULL, ".", &sptr);
	}
	efree(skey);
	return _data;
}

zval *air_config_str_path_get(zval *data, char *path, int path_len) {
	zval *_data = data;
	if(data == NULL){
		_data = air_config_get_data();
	}
	zval **tmp = NULL;
	char *seg = NULL, *sptr = NULL;
	char *skey = estrndup(path, path_len);
	seg = php_strtok_r(skey, ".", &sptr);
	int status = SUCCESS;
	while(seg){
		if(Z_TYPE_P(_data) != IS_ARRAY) {
			_data = NULL;
			break;
		}
		_data = zend_hash_str_find(Z_ARRVAL_P(_data), seg, strlen(seg));
		if(!_data){
			php_error(E_NOTICE, "config path '%s' not found\n", path);
			break;
		}
		seg = php_strtok_r(NULL, ".", &sptr);
	}
	efree(skey);
	return _data;
}
/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_config_get_arginfo, 0, 0, 0)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, default_value)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(air_config_set_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()
/* }}} */

/** {{{ PHP METHODS */
PHP_METHOD(air_config, get) {
	zval *data = air_config_get_data();
	zend_string *key = NULL;
	zval *val = NULL;
	zval *def_val = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "|Sz", &key, &def_val) == FAILURE)
	{
	}
	if(!key){
		RETURN_ZVAL(data, 1, 0);
	}
	val = zend_hash_find(Z_ARRVAL_P(data), key);
	if(!val) {
		if(def_val){
			RETURN_ZVAL(def_val, 1, 0);
		}else{
			RETURN_NULL();
		}
	}else{
		RETURN_ZVAL(val, 1, 0);
	}
}

PHP_METHOD(air_config, path_get) {
	zend_string *key;
	zval *val = NULL;
	zval *def_val = NULL;
	if( zend_parse_parameters(ZEND_NUM_ARGS(), "|Sz", &key, &def_val) == FAILURE )
	{
		AIR_NEW_EXCEPTION(1, "invalid get_path params");
	}
	val = air_config_get_data();
	if(!ZSTR_LEN(key)){
		RETURN_ZVAL(val, 1, 0);
	}
	val = air_config_path_get(val, key);
	if(!val) {
		if(def_val){
			RETURN_ZVAL(def_val, 1, 0);
		}
	}else{
		RETURN_ZVAL(val, 1, 0);
	}
}

PHP_METHOD(air_config, set) {
	zval *data;
	data = zend_read_static_property(air_config_ce, ZEND_STRL("_data"), 1);
	if(!ZVAL_IS_NULL(data)){
		AIR_NEW_EXCEPTION(1, "config can only be set once");
	}
	if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &data) == FAILURE )
	{
	}
	air_config_merge_default(data);
}
/* }}} */

/** {{{ air_config_methods */
zend_function_entry air_config_methods[] = {
	PHP_ME(air_config, get,	air_config_get_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_config, path_get, air_config_get_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_config, set,	air_config_set_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};
/* }}} */

/** {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_config) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\config", air_config_methods);

	air_config_ce = zend_register_internal_class_ex(&ce, NULL);
	air_config_ce->ce_flags |= ZEND_ACC_FINAL;

	zend_declare_property_null(air_config_ce, ZEND_STRL("_data"), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC);

	return SUCCESS;
}
/* }}} */
