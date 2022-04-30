/* Demond Yeld Farms HPP */

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/print.hpp>
#include <eosio/system.hpp>

using namespace std;
using namespace eosio;

class [[eosio::contract("dmdfarms")]] dmdfarms:public eosio::contract 
{
    private:
    const symbol dop_box_lp_symbol;
    const symbol hub_box_lp_symbol;
    const symbol dmd_box_lp_symbol;

    const symbol dop_symbol;
    const symbol hub_symbol;
    const symbol dmd_symbol;

    struct [[eosio::table]] globals 
    {
        name     key;
        uint16_t last_pool_id;

        uint64_t primary_key()const { return key.value; } 
    };
    typedef eosio::multi_index< "globaltable"_n, globals  > globaltable;
 
    struct [[eosio::table]] accounts
    {
        asset    balance;

        uint64_t primary_key()const { return balance.symbol.code().raw(); }
    };
    typedef eosio::multi_index< "accounts"_n, accounts > boxtable;


    struct [[eosio::table]] registered_accounts
    {
        name   owner_account;

        uint64_t boxlptoken_snapshot_amount;
        uint64_t boxlptoken_before_amount;
        uint64_t dmd_claimed_amount;
        uint64_t dmd_unclaimed_amount;

        uint64_t primary_key()const { return owner_account.value; }
    };
    typedef eosio::multi_index< "lptable"_n, registered_accounts > lptable;


    struct [[eosio::table]] pool_stats 
    {
        bool     is_active;

        uint16_t pool_id;

        uint32_t mining_start_time;
        uint32_t halving1_deadline;
        uint32_t halving2_deadline;
        uint32_t halving3_deadline;
        uint32_t halving4_deadline;
        uint32_t last_reward_time;

        uint64_t dmd_mine_qty_remaining;
        uint32_t dmd_issue_frequency;
        uint64_t minimum_lp_tokens;
        uint64_t pool_total_lptokens;

        asset box_asset_symbol;
        std::string pool_name; /* For display purposes */

        uint64_t primary_key()const { return pool_id; } 
    };
    typedef eosio::multi_index< "pooltable"_n, pool_stats > pooltable;

    void inline_transferdmd(eosio::name from, eosio::name to, eosio::asset quantity, std::string memo) const
    {
        struct transfer
        {
            eosio::name from;
            eosio::name to;
            eosio::asset quantity;
            std::string memo;
        };

        eosio::action transfer_action = eosio::action(
            eosio::permission_level(get_self(), "active"_n),
            eosio::name("dmd.efi"), /* Token Contrak */
            eosio::name("transfer"),
            transfer{from, to, quantity, memo});
            transfer_action.send();
    }

    asset get_asset_amount(name owner_account, asset lptoken)
    {   /* Retrieve Defibox LPToken balance for specific account and symbol */
        boxtable accounts("lptoken.defi"_n, owner_account.value);
        auto ac = accounts.find(lptoken.symbol.code().raw());
        return ac->balance;
    }

    public:
    using contract::contract;

    [[eosio::action]]
    void init();

    [[eosio::action]]
    void setpool(uint16_t pool_id, uint32_t dmd_issue_frequency, bool is_active, uint64_t min_lp_tokens, asset box_asset_symbol, string pool_name, uint64_t dmd_mine_qty_remaining);

    [[eosio::action]]
    void registeruser(const name& owner_account, uint16_t pool_id);

    [[eosio::action]]
    void claimrewards(const name& owner_account, uint16_t pool_id);

    [[eosio::action]]
    void issue(uint16_t pool_id);

    [[eosio::action]]
    void purge(uint16_t pool_id);

    [[eosio::action]]
    void dellastpool();

    [[eosio::action]]
    void clearusers(uint16_t pool_id);

    [[eosio::action]]
    void clearpool(uint16_t pool_id);
    
    dmdfarms(name receiver, name code, datastream<const char *> ds):contract(receiver, code, ds), dmd_symbol("DMD", 4), hub_box_lp_symbol("BOXBMZ", 0), 
                                                                                        dop_box_lp_symbol("BOXBMY", 0), dmd_box_lp_symbol("BOXBMU", 0) {}
};
