
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>


using namespace std;

// ==========================================
// 1. ENUMS
// ==========================================

enum class DamageType { Physical, Magic };

enum class StatType {
  Attack,
  MagicPower,
  CritRate,
  CritDamage,
  Lifesteal,
  AttackSpeed,
  CooldownReduction,
  Mana
};

// ==========================================
// 2. STAT STRUCTURES
// ==========================================

// Represents one individual stat.
struct Stat {
  StatType stat_type;
  double stat_value;
};

// Represents all stats belonging to an item.
struct Stats {
  vector<Stat> effects;
};

// ==========================================
// 3. ITEM STRUCTURE
// ==========================================

struct Items {
  string item_name;
  DamageType damage_type;
  string item_effect;
  Stats item_stats;
};

// ==========================================
// 4. CHARACTER BASE STATS
// ==========================================

struct CharacterStats {
  string name;

  // Base stats
  double base_attack = 25;
  double base_magic_power = 10;
  double base_crit_rate = 0.05;
  double base_crit_damage = 0.50;
  double base_lifesteal = 0.0;
  double base_attack_speed = 1.0;
  double base_cooldown_reduction = 0.0;
  double base_mana = 100;

  // Equipped items
  vector<Items> inventory;
};

// ==========================================
// 5. FINAL CHARACTER STATS
// ==========================================

struct FinalStats {
  double attack;
  double magic_power;
  double crit_rate;
  double crit_damage;
  double lifesteal;
  double attack_speed;
  double cooldown_reduction;
  double mana;
};

// ==========================================
// 6. HELPER FUNCTION
// ==========================================

string getStatName(StatType type) {

  switch (type) {
  case StatType::Attack:
    return "Attack";

  case StatType::MagicPower:
    return "Magic Power";

  case StatType::CritRate:
    return "Crit Rate";

  case StatType::CritDamage:
    return "Crit Damage";

  case StatType::Lifesteal:
    return "Lifesteal";

  case StatType::AttackSpeed:
    return "Attack Speed";

  case StatType::CooldownReduction:
    return "Cooldown Reduction";

  case StatType::Mana:
    return "Mana";
  }

  return "Unknown";
}

// ==========================================
// 7. EQUIP ITEM
// ==========================================

void equipItem(CharacterStats &character, const Items &item) {

  character.inventory.push_back(item);

  cout << "\n" << item.item_name << " equipped by " << character.name << "!\n";
}

// ==========================================
// 8. UNEQUIP ITEM
// ==========================================

void unequipItem(CharacterStats &character, const string &item_name) {

  for (int i = 0; i < character.inventory.size(); i++) {

    if (character.inventory[i].item_name == item_name) {

      character.inventory.erase(character.inventory.begin() + i);

      cout << "\n" << item_name << " unequipped!\n";
      return;
    }
  }

  cout << "\nItem not found in inventory.\n";
}

// ==========================================
// 9. CALCULATE FINAL CHARACTER STATS
// ==========================================

FinalStats calculateStats(const CharacterStats &character) {

  // Start with base stats.
  FinalStats final_stats = {character.base_attack,
                            character.base_magic_power,
                            character.base_crit_rate,
                            character.base_crit_damage,
                            character.base_lifesteal,
                            character.base_attack_speed,
                            character.base_cooldown_reduction,
                            character.base_mana};

  // Iterate through equipped items.
  for (const Items &item : character.inventory) {

    // Iterate through each item's effects.
    for (const Stat &stat : item.item_stats.effects) {

      switch (stat.stat_type) {

      case StatType::Attack:
        final_stats.attack += stat.stat_value;
        break;

      case StatType::MagicPower:
        final_stats.magic_power += stat.stat_value;
        break;

      case StatType::CritRate:
        final_stats.crit_rate += stat.stat_value;
        break;

      case StatType::CritDamage:
        final_stats.crit_damage += stat.stat_value;
        break;

      case StatType::Lifesteal:
        final_stats.lifesteal += stat.stat_value;
        break;

      case StatType::AttackSpeed:
        final_stats.attack_speed += stat.stat_value;
        break;

      case StatType::CooldownReduction:
        final_stats.cooldown_reduction += stat.stat_value;
        break;

      case StatType::Mana:
        final_stats.mana += stat.stat_value;
        break;
      }
    }
  }

  return final_stats;
}

// ==========================================
// 10. DISPLAY CHARACTER STATS
// ==========================================

void displayCharacter(const CharacterStats &character) {

  FinalStats stats = calculateStats(character);

  cout << "\n========== CHARACTER ==========\n";

  cout << "Name: " << character.name << endl;

  cout << "\nEquipped Items:\n";

  if (character.inventory.empty()) {
    cout << "No items equipped.\n";
  }

  for (const Items &item : character.inventory) {
    cout << "- " << item.item_name << endl;
  }

  cout << "\n========== FINAL STATS ==========\n";

  cout << fixed << setprecision(2);

  cout << "Attack: " << stats.attack << endl;
  cout << "Magic Power: " << stats.magic_power << endl;

  cout << "Crit Rate: " << stats.crit_rate * 100 << "%\n";
  cout << "Crit Damage: " << stats.crit_damage * 100 << "%\n";

  cout << "Lifesteal: " << stats.lifesteal * 100 << "%\n";

  cout << "Attack Speed: " << stats.attack_speed << endl;

  cout << "Cooldown Reduction: " << stats.cooldown_reduction * 100 << "%\n";

  cout << "Mana: " << stats.mana << endl;

  cout << "=================================\n";
}

// ==========================================
// 11. MAIN
// ==========================================

int main() {

  // Create items.
  Items sword = {"Warrior Sword",
                 DamageType::Physical,
                 "Increases physical damage.",
                 {{{StatType::Attack, 12},
                   {StatType::CritRate, 0.15},
                   {StatType::Lifesteal, 0.08},
                   {StatType::AttackSpeed, 0.10}}}};

  Items magic_wand = {"Magic Wand",
                      DamageType::Magic,
                      "Increases magical damage.",
                      {{{StatType::MagicPower, 25},
                        {StatType::Mana, 100},
                        {StatType::CooldownReduction, 0.10}}}};

  Items critical_blade = {
      "Critical Blade",
      DamageType::Physical,
      "Increases critical damage.",
      {{{StatType::Attack, 20}, {StatType::CritDamage, 0.50}}}};

  // Create character.
  CharacterStats player;

  player.name = "Warrior";

  // Display base character stats.
  cout << "\nBEFORE EQUIPPING ITEMS";
  displayCharacter(player);

  // Equip items.
  equipItem(player, sword);
  equipItem(player, magic_wand);

  // Display updated character stats.
  cout << "\nAFTER EQUIPPING ITEMS";
  displayCharacter(player);

  // Equip another item.
  equipItem(player, critical_blade);

  cout << "\nAFTER EQUIPPING THIRD ITEM";
  displayCharacter(player);

  // Unequip an item.
  unequipItem(player, "Warrior Sword");

  cout << "\nAFTER UNEQUIPPING WARRIOR SWORD";
  displayCharacter(player);

  return 0;
}
