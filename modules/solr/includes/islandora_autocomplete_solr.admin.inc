<?php

/**
 * @file
 *
 * Functions for rendering/vaildating/submitting the Solr subsection of the
 * autocomplet URL form.
 */

/**
 * Returns the form definition for the solr source part of the
 * autocomplete url form.
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
  drupal_add_css(drupal_get_path('module', 'islandora_autocomplete_solr') . '/css/islandora_autocomplete_solr.admin.css');
  // Gt db values.
  $row = islandora_autocomplete_solr_db_get_url_source($url_id);

  $form = array();

  // Reset title.
  $form['#title'] = NULL;

  // Solr field.
  $form['solr_field'] = array(
    '#type' => 'textfield',
    '#description' => t('Solr field from which to return values.'),
    '#title' => t('Solr field'),
    '#default_value' => $row->solr_field ? $row->solr_field : '',
    '#required' => TRUE,
  );
  // NGrams?
  $form['use_ngram'] = array(
    '#type' => 'checkbox',
    '#description' => t('Check if you want to use NGram filter to return autocomplete values.'),
    '#title' => t('Use NGram filter'),
    '#default_value' => $row->use_ngram ? $row->use_ngram : '',
    '#ahah' => array(
      'path' => 'admin/content/autocomplete/ahah/solr',
      'wrapper' => 'ngram-wrapper',
      'effect' => 'fade',
      // 'event' => 'change', default value: does not need to be set explicitly.
    ),
  );
  // NGram field wrapper.
  $form['ngram_field_wrapper'] = array(
    '#type' => 'item',
    '#prefix' => '<div id="ngram-wrapper">',
    '#suffix' => '</div>',
  );

  // NGram field.
  $render_ngram = FALSE;
  if ($form_state['ahah_submission'] == TRUE) {
    if ($form_state['values']['use_ngram'] == 1) {
      $render_ngram = TRUE;
    }
  }
  elseif ($row->use_ngram == 1) {
    $render_ngram = TRUE;
  }
  if ($render_ngram) {
    $form['ngram_field_wrapper']['ngram_field'] = array(
      '#type' => 'textfield',
      '#description' => t('NGram field on which to query.'),
      '#title' => t('NGram field'),
      '#default_value' => $row->ngram_field ? $row->ngram_field : '',
    );
  }
  // Comma separated values.
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

  // Don't use sumbit functions when ahah is called.
  if ($form_state['ahah_submission'] !== TRUE) {
    module_load_include('inc', 'islandora_autocomplete_solr', 'includes/islandora_autocomplete_solr.db');
    extract($form_state['values'], EXTR_SKIP);
    // Solr field.
    if (!empty($solr_field)) {
       // File already existed.
      if (isset($url_id)) {
        $source = islandora_autocomplete_solr_db_get_url_source($url_id);
        // Create values array.
        $values = array(
          'solr_field' => $solr_field ? $solr_field : '',
          'comma_separated' => $comma_separated,
          'use_ngram' => $use_ngram,
          'ngram_field' => $ngram_field ? $ngram_field : '',
        );
        // Source exists, update it.
        if ($source) {
          islandora_autocomplete_solr_db_update_url_source($source->id, $url_id, $values);
          // Updated Source.
          return;
        }
        // Add new source.
        islandora_autocomplete_solr_db_add_url_source($url_id, $values);
      }
    }
  }
}

/**
 * islandora autocomplete solr ahah callback
 */
function islandora_autocomplete_solr_admin_ahah() {
  // Include original form file.
  module_load_include('inc', 'islandora_autocomplete', 'includes/islandora_autocomplete.admin');
  // Generic AHAH fiddling.
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  $form = form_get_cache($form_build_id, $form_state);
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  // Enable the submit/validate handlers to determine whether AHAH-submittted.
  $form_state['ahah_submission'] = TRUE;
  $form['#programmed'] = $form['#redirect'] = FALSE;
  drupal_process_form($form_id, $form, $form_state);
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);

  // NGram wrapper.
  $ngram_wrapper = $form['source_form']['ngram_field_wrapper'];

  // Remove the prefix/suffix wrapper so we don't double it up.
  unset($ngram_wrapper['#prefix'], $ngram_wrapper['#suffix']);

  // Render the output.
  //  $output = theme('status_messages');
  $output = drupal_render($ngram_wrapper);

  // Final rendering callback.
  drupal_json(array('status' => TRUE, 'data' => $output));
  exit();
}
