#include <scorum/chain/dbs_reward.hpp>
#include <scorum/chain/database.hpp>
#include <scorum/chain/pool/reward_pool.hpp>

namespace scorum {
namespace chain {

dbs_reward::dbs_reward(database& db)
    : _base_type(db)
{
}

const reward_pool_object& dbs_reward::create_pool(const asset& initial_supply)
{
    FC_ASSERT(db_impl().find<reward_pool_object>() == nullptr, "recreation of reward_pool_object is not allowed");

    return db_impl().create<reward_pool_object>([&](reward_pool_object& rp) {
        // clang-format off
        rp.balance = initial_supply;
        rp.current_per_block_reward = asset(rp.balance.amount / (SCORUM_BLOCKS_PER_DAY * (SCORUM_GUARANTED_REWARD_SUPPLY_PERIOD_IN_DAYS + 1)));
        // clang-format on
    });
}

const reward_pool_object& dbs_reward::get_pool() const
{
    return db_impl().get<reward_pool_object>();
}

const asset& dbs_reward::increase_pool_ballance(const asset& delta)
{
    const auto& pool = get_pool();

    db_impl().modify(pool, [&](reward_pool_object& pool) {
        switch (delta.symbol)
        {
        case SCORUM_SYMBOL:
            FC_ASSERT(delta >= asset(0));
            pool.balance += delta;
            break;
        default:
            FC_ASSERT(false, "invalid symbol");
        }
    });

    return pool.balance;
}

// clang-format off
const asset& dbs_reward::take_block_reward()
{
    const auto& pool = get_pool();

    FC_ASSERT(pool.current_per_block_reward > asset(0));

    if (pool.balance > asset(pool.current_per_block_reward.amount * SCORUM_BLOCKS_PER_DAY * SCORUM_REWARD_INCREASE_THRESHOLD_IN_DAYS))
    {
        // recalculate
        db_impl().modify(pool, [&](reward_pool_object& pool) { 
            pool.current_per_block_reward += asset((pool.current_per_block_reward.amount * SCORUM_ADJUST_REWARD_PERCENT) / 100);
        });
    }
    else if (pool.balance < asset(pool.current_per_block_reward.amount * SCORUM_BLOCKS_PER_DAY * SCORUM_GUARANTED_REWARD_SUPPLY_PERIOD_IN_DAYS))
    {
        // recalculate
        db_impl().modify(pool, [&](reward_pool_object& pool) {
            pool.current_per_block_reward -= asset((pool.current_per_block_reward.amount * SCORUM_ADJUST_REWARD_PERCENT) / 100);
        });
    }
    else
    {
        // use current_perblock_reward
    }

    db_impl().modify(pool, [&](reward_pool_object& pool) { pool.balance -= pool.current_per_block_reward; });

    return pool.current_per_block_reward;
}
// clang-format on

} // namespace chain
} // namespace scorum
