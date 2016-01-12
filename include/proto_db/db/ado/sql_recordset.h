#ifndef SQL_RECORDSET_H_
#define SQL_RECORDSET_H_

#include <memory>
#include <string>
#include <comutil.h>
#import "./msado15.dll" no_namespace rename("EOF","adoEOF")

namespace sql {
class RecordSet {
public:

    explicit RecordSet(_RecordsetPtr& record_set, _CommandPtr& command_ptr);

    ~RecordSet();

    HRESULT get_last_error() const;

    //  ����Ƿ�EOF
    bool is_eof();

    bool is_empty();

    //  ��һ����¼
    bool move_next();

    _variant_t get_field_value(const std::basic_string<TCHAR>& name);

    _variant_t get_param_value(const std::basic_string<TCHAR>& name);

    long get_field_count();

    std::basic_string<TCHAR> get_field_name(long index);


    //  ����_RecordsetPtrָ�룬������������
    _RecordsetPtr native_record_set();

private:

    //  ��ȡ��ǰ��¼��ָ���ֶ�������ֵ
    bool get_field_value(const std::basic_string<TCHAR>& name, _variant_t& value);

    //  ��ȡ�洢����ָ��������ֵ
    bool get_param_value(const std::basic_string<TCHAR>& name, _variant_t& value);

    void set_last_error(HRESULT hr);
    void set_last_error(const _com_error& e);

private:
    HRESULT hr_;
    _CommandPtr command_ptr_;
    _RecordsetPtr record_set_ptr_;
    std::basic_string<TCHAR> error_string_;
};

typedef std::shared_ptr<RecordSet> record_set_ptr;

} // namespace

#endif  // SQL_RECORDSET_H_