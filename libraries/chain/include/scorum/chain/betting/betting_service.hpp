#pragma once
#include <scorum/protocol/betting/market.hpp>
#include <scorum/chain/schema/scorum_object_types.hpp>
#include <scorum/utils/any_range.hpp>

#include <scorum/protocol/types.hpp>

#include <vector>
#include <functional>

namespace scorum {
namespace chain {

struct bet_data;
enum class pending_bet_kind : uint8_t;

using scorum::protocol::market_type;

struct account_service_i;
struct database_virtual_operations_emmiter_i;

class dynamic_global_property_object;
class betting_property_object;
class pending_bet_object;
class matched_bet_object;

namespace dba {
template <typename> class db_accessor;
}

struct betting_service_i
{
    virtual ~betting_service_i();

    virtual bool is_betting_moderator(const account_name_type& account_name) const = 0;

    virtual const pending_bet_object& create_pending_bet(const account_name_type& better,
                                                         const protocol::asset& stake,
                                                         protocol::odds odds,
                                                         const protocol::wincase_type& wincase,
                                                         uuid_type game_uuid,
                                                         uuid_type bet_uuid,
                                                         pending_bet_kind kind)
        = 0;

    virtual void cancel_game(uuid_type game_uuid) = 0;
    virtual void cancel_bets(uuid_type game_uuid) = 0;
    virtual void cancel_bets(uuid_type game_uuid, fc::time_point_sec created_after) = 0;
    virtual void cancel_bets(uuid_type game_uuid, const fc::flat_set<market_type>& cancelled_markets) = 0;

    virtual void cancel_pending_bet(pending_bet_id_type id) = 0;
    virtual void cancel_pending_bets(uuid_type game_uuid) = 0;
    virtual void cancel_pending_bets(uuid_type game_uuid, pending_bet_kind kind) = 0;
    virtual void cancel_pending_bets(utils::bidir_range<const pending_bet_object> bets) = 0;

    virtual void cancel_matched_bets(uuid_type game_uuid) = 0;
    virtual void cancel_matched_bets(utils::bidir_range<const matched_bet_object> bets, uuid_type game_uuid) = 0;
};

class betting_service : public betting_service_i
{
public:
    betting_service(account_service_i&,
                    database_virtual_operations_emmiter_i&,
                    dba::db_accessor<betting_property_object>&,
                    dba::db_accessor<matched_bet_object>&,
                    dba::db_accessor<pending_bet_object>&,
                    dba::db_accessor<game_object>&,
                    dba::db_accessor<dynamic_global_property_object>&,
                    dba::db_accessor<bet_uuid_history_object>&);

    bool is_betting_moderator(const account_name_type& account_name) const override;

    const pending_bet_object& create_pending_bet(const account_name_type& better,
                                                 const protocol::asset& stake,
                                                 protocol::odds odds,
                                                 const protocol::wincase_type& wincase,
                                                 uuid_type game_uuid,
                                                 uuid_type bet_uuid,
                                                 pending_bet_kind kind) override;

    void cancel_game(uuid_type game_uuid) override;
    void cancel_bets(uuid_type game_uuid) override;
    void cancel_bets(uuid_type game_uuid, fc::time_point_sec created_after) override;
    void cancel_bets(uuid_type game_uuid, const fc::flat_set<market_type>& cancelled_markets) override;

    void cancel_pending_bet(pending_bet_id_type id) override;
    void cancel_pending_bets(uuid_type game_uuid) override;
    void cancel_pending_bets(uuid_type game_uuid, pending_bet_kind kind) override;
    void cancel_pending_bets(utils::bidir_range<const pending_bet_object> bets) override;

    void cancel_matched_bets(uuid_type game_uuid) override;
    void cancel_matched_bets(utils::bidir_range<const matched_bet_object> bets, uuid_type game_uuid) override;

private:
    void cancel_pending_bet(const pending_bet_object& bet, uuid_type game_uuid);
    void cancel_matched_bet(const matched_bet_object& bet, uuid_type game_uuid);
    void return_bet(const bet_data& bet, uuid_type game_uuid);
    void restore_pending_bet(const bet_data& bet, uuid_type game_uuid);
    void push_matched_bet_cancelled_op(const bet_data& bet, uuid_type game_uuid);
    void push_pending_bet_cancelled_op(const bet_data& bet, uuid_type game_uuid);

    account_service_i& _account_svc;
    database_virtual_operations_emmiter_i& _virt_op_emitter;
    dba::db_accessor<betting_property_object>& _betting_property_dba;
    dba::db_accessor<matched_bet_object>& _matched_bet_dba;
    dba::db_accessor<pending_bet_object>& _pending_bet_dba;
    dba::db_accessor<game_object>& _game_dba;
    dba::db_accessor<dynamic_global_property_object>& _dprop_dba;
    dba::db_accessor<bet_uuid_history_object>& _uuid_hist_dba;
};
}
}
