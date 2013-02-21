<?php
  /**
   * @file
   * registration_commerce.rules_defaults.inc
   */

  /**
   * Implements hook_default_rules_configuration().
   */
function registration_commerce_default_rules_configuration() {

  // Add a reaction rule to set the price of a registration product based on the registration itself
  $rule = rules_reaction_rule();

  $rule->label = t('Set registration product price from referenced registration');
  $rule->active = TRUE;
  $rule->weight = 10;

  $rule
    ->event('commerce_product_calculate_sell_price')
    ->condition('entity_has_field', array(
    'entity:select' => 'commerce-line-item',
    'field' => REGISTRATION_COMMERCE_FIELD,
  ))
    ->condition('data_is', array(
    'data:select' => 'commerce-line-item:registration-commerce-reg:price',
    'op' => '>',
    'value' => '0',
  ))
    ->action('commerce_line_item_unit_price_amount', array(
    'commerce_line_item:select' => 'commerce-line-item',
    'amount:select' => 'commerce-line-item:registration-commerce-reg:price',
    'component_name' => 'base_price',
    'round_mode' => '1',
  ));

  $rules['rules_registration_commerce_product_price_from_registration'] = $rule;

  return $rules;
}