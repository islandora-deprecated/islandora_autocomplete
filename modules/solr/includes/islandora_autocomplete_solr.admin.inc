<?php

/**
 * @file
 *
 * Functions for rendering/vaildating/submitting the Solr subsection of the autocomplet URL form.
 */

/**
 * Returns the form definition for the solr source part of the autocomplete url form.
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
dsm($form_state);
dsm($url_id);
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
      // 'event' => 'change', // default value: does not need to be set explicitly.
    ),
  );
  // NGram field wrapper
  $form['ngram_field_wrapper'] = array(
    '#type' => 'item',
    '#prefix' => '<div id="ngram-wrapper">',
    '#suffix' => '</div>',
  );
  
  // NGram field
  if (!empty($form_state['values']['use_ngram']) OR !empty($row->use_ngram)) {
    $form['ngram_field_wrapper']['ngram_field'] = array(
      '#type' => 'textfield',
      '#description' => t('NGram field on which to query.'),
      '#title' => t('NGram field'),
      '#default_value' => $row->ngram_field ? $row->ngram_field : '',
    );
  }
  // comma separated values
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
      // create values array
      $values = array(
        'solr_field' => $solr_field,
        'comma_separated' => $comma_separated,
        'use_ngram' => $use_ngram,
        'ngram_field' => $ngram_field,
      );
      if ($source) { // Source exists, update it.
        islandora_autocomplete_solr_db_update_url_source($source->id, $url_id, $values);
        return; // Updated Source.
      } // Add new source.
      islandora_autocomplete_solr_db_add_url_source($url_id, $values);
    }
  }
}


function islandora_autocomplete_solr_admin_ahah() {
  $form_state = array('storage' => NULL, 'submitted' => FALSE);  // , 'post' => $_POST
  $form_build_id = $_POST['form_build_id'];
  $form = form_get_cache($form_build_id, $form_state);
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  // Enable the submit/validate handlers to determine whether AHAH-submittted.
  $form_state['ahah_submission'] = TRUE;
  $form['#programmed'] = $form['#redirect'] = FALSE;
  drupal_process_form($form_id, $form, $form_state);
dsm($form);
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);
dsm($form);
//  $ngram_wrapper = $form['ngram_field_wrapper'];

  // Remove the prefix/suffix wrapper so we don't double it up.
//  unset($ngram_wrapper['#prefix'], $ngram_wrapper['#suffix']);

  // Render the output.
  $output = theme('status_messages');
  $output .= drupal_render($ngram_wrapper);

  // Final rendering callback.
  drupal_json(array('status' => TRUE, 'data' => $output));
  exit();


/*
  if (!isset($_REQUEST['form_build_id'])) { // Request is not valid
    header("HTTP/1.0 200 OK", FALSE, 200);
    exit();
  }
  $form_build_id = $_REQUEST['form_build_id'];
  $form_state = array('storage' => NULL, 'submitted' => FALSE, 'post' => $_POST);
  if (!$form = form_get_cache($form_build_id, $form_state)) {
    header("HTTP/1.0 200 OK", FALSE, 200);
    exit();
  }
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  $form['#programmed'] = $form['#redirect'] = FALSE;
  drupal_process_form($form_id, $form, $form_state);
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);
  drupal_get_messages('error'); // Clear errors.
  $html = '';
  if (isset($form)) {
    unset($form['source_form']['#prefix'], $form['source_form']['#suffix']);
    $html = drupal_render($form['source_form']);
  }
  $javascript = drupal_add_js(NULL, NULL, 'header');
  $settings = call_user_func_array('array_merge_recursive', $javascript['setting']);
  unset($settings['ahah']['']);
  drupal_json(array(
    'status' => TRUE,
    'data' => theme('status') . $html,
    'settings' => $settings,
  ));
*/
  
}



/**
 * Does the very standard things that must be done in any normal callback.
 */
function islandora_autocomplete_solr_callback_helper() {

}


