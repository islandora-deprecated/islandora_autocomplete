<?php

/**
 * @file
 *
 * Functions for rendering/vaildating/submitting the Solr subsection of the autocomplet URL form.
 */

/**
 * Returns the form definition for the csv source part of the autocomplete url form.
 *
 * @param array $form_state
 *   The drupal form state.
 * @param int $url_id
 *   The id of the URL being edited, It is null when creating a new url.
 *
 * @return array
 *   The csv source part of the autocomplete url form.
 */
function islandora_autocomplete_solr_form(array &$form_state, $url_id = NULL) {
  module_load_include('inc', 'islandora_autocomplete_solr', 'includes/islandora_autocomplete_solr.db');

  $row = islandora_autocomplete_solr_db_get_url_source($url_id);

  $form = array();

  // reset title
  $form['#title'] = NULL;

  // solr field
  $form['solr_field'] = array(
    '#type' => 'textfield',
    '#description' => t('Solr field from which to return values.'),
    '#title' => t('Solr field'),
    '#default_value' => $row->solr_field ? $row->solr_field : '',
  );
  // @TODO: comma separated values?
  $form['comma_separated'] = array(
    '#type' => 'checkbox',
    '#description' => t('If checked, you will be able to autocomplete on different values separated by a comma.'),
    '#title' => t('Comma separated'),
    '#default_value' => $row->comma_separated ? $row->comma_separated : '',
  );
  
  // @TODO: autocomplete style: NGrams, facet prefix (not sure if this is necessary)

  return $form;
}

/**
 * Validate the source portion of the submitted form.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_autocomplete_solr_form_validate(array $form, array &$form_state) {

}

/**
 * Submits the source portion of the form.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_autocomplete_solr_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_autocomplete_solr', 'includes/islandora_autocomplete_solr.db');
  extract($form_state['values'], EXTR_SKIP);
  if (!empty($solr_field)) { // solr field
    if (isset($url_id)) { // File already existed
      $source = islandora_autocomplete_solr_db_get_url_source($url_id);
      if ($source) { // Source exists, update it.
        islandora_autocomplete_solr_db_update_url_source($source->id, $url_id, $solr_field, $comma_separated);
        return; // Updated Source.
      } // Add new source.
      islandora_autocomplete_solr_db_add_url_source($url_id, $solr_field, $comma_separated);
    }
  }
}